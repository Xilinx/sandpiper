/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#if ENABLE_MATLAB_TEST
	#define MATLAB_TCP_MULTI_CUBE
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include <chrono>
#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include "adf.h"
#ifdef MATLAB_TCP_MULTI_CUBE
#include "mem_dump.h"
#include "cube_params.h"
#include "server.hpp"
#include "serial_processor.hpp"
#endif
#include <thread>
#include <atomic>

#include <experimental/xrt_aie.h>
#include <experimental/xrt_kernel.h>
#include <experimental/xrt_bo.h>

#ifndef MATLAB_TCP_MULTI_CUBE
#include "input_0_data.h"
#include "input_1_data.h"
#include "input_2_data.h"
#include "input_3_data.h"
#include "weightAppOut0_data.h"
#include "weightAppOut1_data.h"
#include "weightAppOut2_data.h"
#include "weightAppOut3_data.h"
#endif

#include "test_common.hpp"
#include <unistd.h>

#define NUM_INPUT_SAMPLES 40000
#define NUM_CUBE_SAMPLES  (4*40000)
#define NUM_OUTPUT_SAMPLES 1000
#define NUM_INPUT_ECI_MTX 1600

//Each of NUM_ITERATIONS processes 10 cuboids data, to enable infinite iterations set INFINITE macro below to "true"
#define NUM_ITERATIONS 1
#define NUM_CUBOIDS 1

#ifndef MATLAB_TCP_MULTI_CUBE
#define INFINITE false
#else
#define INFINITE true 
#endif
#define W_DATAMOVER_INSTS 4
#define R_DATAMOVER_INSTS 4
#define W_CFG_COUNT 4
#define R_CFG_COUNT 4
#define SHARED_BUFFERS 8
#define CFG_SIZE 9
#define CH_SELECTOR_INSTS 4

#define PING_BUFFER_FLAG 0
#define PONG_BUFFER_FLAG 1

#define N 4*40000
//#define N (1 << 20)
#define M 100

#ifndef MATLAB_TCP_MULTI_CUBE
std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point rdma_start;
std::chrono::high_resolution_clock::time_point wdma_start;
std::chrono::high_resolution_clock::time_point rdma_end;
std::chrono::high_resolution_clock::time_point wdma_end;
std::chrono::duration<double> wdma_time[NUM_ITERATIONS];
std::chrono::duration<double> rdma_time[NUM_ITERATIONS];
#endif
int group_id;


using namespace std;

#ifndef MATLAB_TCP_MULTI_CUBE
const float NORMALIZED_DOPPLER = 0.1639799327; // 0.1027;
const float NORMALIZED_SPATIAL = 0.000000000;
#else
int curr_cuboid = 0;
char *xclbinFilename;
#endif

atomic<int> cmpl_rdma_ping_count (0);
atomic<int> cmpl_rdma_pong_count (0);

atomic<int> cmpl_wdma_ping_count (0);
atomic<int> cmpl_wdma_pong_count (0);

struct DataBufferStore 
{
    public:
	xrtBufferHandle data_bo_hdl;

	uint64_t *data_bo_mapped;

	size_t data_size_in_bytes;

	void initData(xrtBufferHandle parent_data_handle, size_t offset) 
	{
	    data_size_in_bytes = N/4 * sizeof(uint64_t);
	    data_bo_hdl = xrtBOSubAlloc(parent_data_handle, data_size_in_bytes, offset);
	}

	void clear() 
	{
	    xrtBOFree(data_bo_hdl);
	}

	xrtBufferHandle get_data_bo_hdl() const {
	    return data_bo_hdl;
	}
};

struct CfgBufferStore 
{
    private:
	xrtBufferHandle cfg_bo_hdl;

	uint64_t *cfg_bo_mapped;

	size_t cfg_size_in_bytes;

    public:
	void initCFG(xrtDeviceHandle dhdl, uint64_t cfg_arr[]) 
	{
	    cfg_size_in_bytes = CFG_SIZE * sizeof(uint64_t);
	    cfg_bo_hdl = xrtBOAlloc(dhdl, cfg_size_in_bytes, 0, group_id);
	    cfg_bo_mapped = reinterpret_cast<uint64_t *>(xrtBOMap(cfg_bo_hdl));
	    for(int i = 0; i < CFG_SIZE; i++) {
		cfg_bo_mapped[i] = cfg_arr[i];         // Start address is 0
	    }                                           // Contiguous address increment
	}

	void clear() 
	{
	    xrtBOFree(cfg_bo_hdl);
	}

	xrtBufferHandle get_cfg_bo_hdl() const {
	    return cfg_bo_hdl;
	}

	size_t get_cfg_size_in_bytes() const {
	    return cfg_size_in_bytes;
	}

};

class wdma
{
    private:
	xrtKernelHandle datamover_khdl;
	xrtRunHandle datamover_rhdl;

    public:
	void open(xrtDeviceHandle dhdl, const axlf *top, char insts)
	{
	    std::string datamover_obj_str = "wdma:{wdma_"+to_string(insts)+ "}";
	    const char *datamover_obj = datamover_obj_str.c_str();

	    //////////////////////////////////////////
	    // Data Mover IP open
	    //////////////////////////////////////////

	    datamover_khdl = xrtPLKernelOpen(dhdl, top->m_header.uuid, datamover_obj); //
	    datamover_rhdl = xrtRunOpen(datamover_khdl);
		group_id = xrtKernelArgGroupId(datamover_khdl, 1);
	}


	void init(xrtDeviceHandle dhdl, const axlf *top, char insts, DataBufferStore* data_store, DataBufferStore* data_store_two, CfgBufferStore& cfg_store)
	{
	    int rval = xrtRunSetArg(datamover_rhdl, 1, data_store[0].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 2, data_store[1].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 3, data_store[2].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 4, data_store[3].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 5, data_store_two[0].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 6, data_store_two[1].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 7, data_store_two[2].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 8, data_store_two[3].get_data_bo_hdl());
	    rval = xrtRunSetArg(datamover_rhdl, 9, NUM_CUBOIDS);
	    rval = xrtRunSetArg(datamover_rhdl, 0, cfg_store.get_cfg_bo_hdl());
	    //printf ("Run time args passed to wdma DMAs\n");
	    xrtBOSync(cfg_store.get_cfg_bo_hdl(), XCL_BO_SYNC_BO_TO_DEVICE, cfg_store.get_cfg_size_in_bytes(), 0);
	}

	void run(void)
	{
	    xrtRunStart(datamover_rhdl);
	    printf("wdma : datamover is running...\n");
	}

	void waitTo_complete(int i /* 0 ping, 1 pong */)
	{
	    //std::cout << "wdma : datamover Kernel in progress "<< std::endl;
	    auto state = xrtRunWait(datamover_rhdl);

	    if (i == PING_BUFFER_FLAG) {
		cmpl_wdma_ping_count++;
	    } else if (i == PONG_BUFFER_FLAG) {
		cmpl_wdma_pong_count++;
	    }

	    //std::cout << "wdma : datamover Kernel completed with status(" << state << ")\n";
	    assert(state == 4 && "wdma completed with wrong return code");
	}

	void close()
	{
	    xrtRunClose(datamover_rhdl);
	    xrtKernelClose(datamover_khdl);
	}

};

class rdma
{
    private:
	xrtKernelHandle datamover_khdl;
	xrtRunHandle datamover_rhdl;

    public:
	void open(xrtDeviceHandle dhdl, const axlf *top, char insts)
	{
	    std::string datamover_obj_str = "rdma:{rdma_"+to_string(insts)+ "}";
	    const char *datamover_obj = datamover_obj_str.c_str();

	    //////////////////////////////////////////
	    // Data Mover IP open
	    //////////////////////////////////////////

	    datamover_khdl = xrtPLKernelOpen(dhdl, top->m_header.uuid, datamover_obj);
	    datamover_rhdl = xrtRunOpen(datamover_khdl);
	}


	void init(xrtDeviceHandle dhdl, const axlf *top, char insts, xrtBufferHandle& data_bo_hdl, xrtBufferHandle& data_bo_hdl2, CfgBufferStore& cfg_store)
	{
	    int rval = xrtRunSetArg(datamover_rhdl, 1, data_bo_hdl);
	    rval = xrtRunSetArg(datamover_rhdl, 2, data_bo_hdl);
	    rval = xrtRunSetArg(datamover_rhdl, 3, data_bo_hdl);
	    rval = xrtRunSetArg(datamover_rhdl, 4, data_bo_hdl);
	    rval = xrtRunSetArg(datamover_rhdl, 5, data_bo_hdl2);
	    rval = xrtRunSetArg(datamover_rhdl, 6, data_bo_hdl2);
	    rval = xrtRunSetArg(datamover_rhdl, 7, data_bo_hdl2);
	    rval = xrtRunSetArg(datamover_rhdl, 8, data_bo_hdl2);
	    rval = xrtRunSetArg(datamover_rhdl, 9, NUM_CUBOIDS);
	    rval = xrtRunSetArg(datamover_rhdl, 0, cfg_store.get_cfg_bo_hdl());
	    //printf ("Run time args passed to rdma DMAs\n");
	    xrtBOSync(cfg_store.get_cfg_bo_hdl(), XCL_BO_SYNC_BO_TO_DEVICE, cfg_store.get_cfg_size_in_bytes(), 0);
	}

	void run(void)
	{
	    xrtRunStart(datamover_rhdl);
	    printf("rdma : datamover is running...\n");
	}

	void waitTo_complete(int i /* 0 ping, 1 pong */)
	{
	    //std::cout << "rdma : datamover Kernel in progress "<< std::endl;
	    auto state = xrtRunWait(datamover_rhdl);

	    if (i == PING_BUFFER_FLAG) {
		cmpl_rdma_ping_count++;
	    } else if (i == PONG_BUFFER_FLAG) {
		cmpl_rdma_pong_count++;
	    }

	    //std::cout << "rdma : datamover Kernel completed with status(" << state << ")\n";
	    assert(state == 4 && "rdma completed with wrong return code");
	}

	void getDataFromDevice(DataBufferStore& store)
	{
	    xrtBOSync(store.data_bo_hdl, XCL_BO_SYNC_BO_FROM_DEVICE, store.data_size_in_bytes, 0);
	    //std::cout << "RDMA : data: "<< std::hex  << store.data_bo_mapped[0] << std::endl;
	}

	void close()
	{
	    xrtRunClose(datamover_rhdl);
	    xrtKernelClose(datamover_khdl);
	}

};

class MM2S
{
    private:
	xrtKernelHandle mm2s_khdl[W_DATAMOVER_INSTS];
	xrtRunHandle mm2s_rhdl[W_DATAMOVER_INSTS];
	xrtBufferHandle mm2s_bohdl[W_DATAMOVER_INSTS];
	uint32_t* mm2s_mapped[W_DATAMOVER_INSTS];

    public:
	void open(xrtDeviceHandle dhdl, const axlf *top)
	{

	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		std::stringstream s;
		s << "mm2s:{mm2s_" << i << "}"; 
		mm2s_khdl[i] = xrtPLKernelOpen(dhdl, top->m_header.uuid, s.str().c_str());
		s.str("");
	    }
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		mm2s_rhdl[i] = xrtRunOpen(mm2s_khdl[i]);
	    }
	}

	void setArgs(xrtDeviceHandle dhdl, const axlf *top)
	{
	    //Pass the buffer and number of samples to MM2S.
	    //Since MM2S bus is 64-bit, and samples are 32-bit, Numer of data items is passed as
	    //NUM_INPUT_SAMPLES/2
	    int rval;
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		rval = xrtRunSetArg(mm2s_rhdl[i], 0, mm2s_bohdl[i]);
		rval = xrtRunSetArg(mm2s_rhdl[i], 2, (NUM_CUBOIDS*NUM_INPUT_SAMPLES)/2);
	    }
	    printf ("Run time args passed to mm2s DMAs\n");
	}


	void allocBuffers(xrtDeviceHandle dhdl)
	{
	    //Allocate BOs (buffer objects) of requested size with appropriate flags
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		mm2s_bohdl[i] = xrtBOAlloc(dhdl, sizeof(uint32_t)*NUM_INPUT_SAMPLES*NUM_CUBOIDS, 0,  0);
	    }

	    //printf ("Allocated Buffer handles to mm2s and s2mm\n");

	    //Memory map BOs into user's address space (DDR Memory)
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		mm2s_mapped[i] = reinterpret_cast<uint32_t*>(xrtBOMap(mm2s_bohdl[i]));
	    }
	}

#ifndef MATLAB_TCP_MULTI_CUBE
	void writeData() {
	    for(unsigned int i = 0; i < NUM_INPUT_SAMPLES*NUM_CUBOIDS; i++) {
		// Replace input_*_data[*] arrays below w/ cuboid data (for all 10) from Matlab
		mm2s_mapped[0][i] = input_0_data[i]; // Store data in DDR
		mm2s_mapped[1][i] = input_1_data[i]; 
		mm2s_mapped[2][i] = input_2_data[i]; 
		mm2s_mapped[3][i] = input_3_data[i]; 
	    }

	    printf ("MM2S: cuboid data is written to DDR\n");
	}
#else
    void writeData(uint32_t *cube_data) {

	    for(unsigned int i = 0; i < NUM_CUBOIDS; i++)
        {
            // Repeat usage of the first (0th) cuboid for all 10 cuboids sent from Matlab
            //uint32_t *cube_p  = &cube_data[NUM_CUBE_SAMPLES * 0];
            uint32_t *cube_p  = &cube_data[NUM_CUBE_SAMPLES * curr_cuboid];

            uint32_t *mm2s0_p = mm2s_mapped[0] + (NUM_INPUT_SAMPLES * i);
            uint32_t *mm2s1_p = mm2s_mapped[1] + (NUM_INPUT_SAMPLES * i);
            uint32_t *mm2s2_p = mm2s_mapped[2] + (NUM_INPUT_SAMPLES * i);
            uint32_t *mm2s3_p = mm2s_mapped[3] + (NUM_INPUT_SAMPLES * i);

            //printf("CUBOID %d INPUT DATA:\n", curr_cuboid + 1);
            //for (int i = 0; i < 20; i++)
            //{
            //    int16_t data = ((int16_t *) cube_p)[i];
            //    printf("%d : %d\n", i, data);
            //}

            memcpy(mm2s0_p, &cube_p[NUM_INPUT_SAMPLES * 0], NUM_INPUT_SAMPLES * sizeof(uint32_t));
            memcpy(mm2s1_p, &cube_p[NUM_INPUT_SAMPLES * 1], NUM_INPUT_SAMPLES * sizeof(uint32_t));
            memcpy(mm2s2_p, &cube_p[NUM_INPUT_SAMPLES * 2], NUM_INPUT_SAMPLES * sizeof(uint32_t));
            memcpy(mm2s3_p, &cube_p[NUM_INPUT_SAMPLES * 3], NUM_INPUT_SAMPLES * sizeof(uint32_t));

            /*printf("Cube input 0:\n");
            mem_dump((uint8_t *)mm2s0_p, 100);
            printf("Cube input 1:\n");
            mem_dump((uint8_t *)mm2s1_p, 100);
            printf("Cube input 2:\n");
            mem_dump((uint8_t *)mm2s2_p, 100);
            printf("Cube input 3:\n");
            mem_dump((uint8_t *)mm2s3_p, 100);*/
        }

	    printf ("cuboid data is written to DDR\n");
	}
#endif	

	void clear() {
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		xrtBOFree(mm2s_bohdl[i]);
	    }
	}

	void run() {

	    // Execute the Data Mover Kernels 
	    //Create a kernel run handle to start each data mover pl kernel
	    //mm2s kernels moves data from DDR to AI Engine

	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		xrtRunStart(mm2s_rhdl[i]);
	    }

	    printf ("mm2s running\n");
	}

	void close(void)
	{
	    //printf ("Close mm2s/s2mm \n");

	    //Wait for hls DMA execution to finish
	    //Close the run handles obtained by in xrtKernelRun()  
	    //Close opened kernel handles obtained from xrtPLKernelOpen()
	    for (int i = 0; i < W_DATAMOVER_INSTS; ++i) {
		xrtRunClose(mm2s_rhdl[i]);
		xrtKernelClose(mm2s_khdl[i]);
	    }
	}

	void waitTo_complete(void)
	{
	    while((ERT_CMD_STATE_COMPLETED != xrtRunState(mm2s_rhdl[0])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(mm2s_rhdl[1])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(mm2s_rhdl[2])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(mm2s_rhdl[3])));
	    printf ("MM2S COMPLETED\n");
	}

};

int compare(float out, float gld) {
    if (1 == isnan(out)) {
        std::cout << "out is abnorman, out=" << out << std::endl;
        return 1;
    } else {
        if (abs(gld) > 1.0) {
            if ((abs(out - gld) / abs(gld)) > 0.000001) {
                return 1;
            }
        } else {
            if (abs(out - gld) > 0.000001) {
                return 1;
            }
        }
    }
    return 0;
}

class S2MM
{
    private:
	int errCnt;
	xrtKernelHandle s2mm_khdl[R_DATAMOVER_INSTS];
	xrtRunHandle s2mm_rhdl[R_DATAMOVER_INSTS];
	xrtBufferHandle s2mm_bohdl[R_DATAMOVER_INSTS];
	cfloat* s2mm_mapped[R_DATAMOVER_INSTS];
#ifdef MATLAB_TCP_MULTI_CUBE
	cfloat result[R_DATAMOVER_INSTS][NUM_OUTPUT_SAMPLES];
#endif

    public:
	void open(xrtDeviceHandle dhdl, const axlf *top)
	{
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		std::stringstream s;
		s << "s2mm:{s2mm_" << i << "}"; 
		s2mm_khdl[i] = xrtPLKernelOpen(dhdl, top->m_header.uuid, s.str().c_str());
		s.str("");
	    }

	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		s2mm_rhdl[i] = xrtRunOpen(s2mm_khdl[i]);
	    }
	}

	void setArgs(xrtDeviceHandle dhdl, const axlf *top)
	{
	    //Pass the buffer and number of samples to MM2S.
	    //Since MM2S bus is 64-bit, and samples are 32-bit, Numer of data items is passed as
	    //NUM_INPUT_SAMPLES/2
	    int rval;
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		rval = xrtRunSetArg(s2mm_rhdl[i], 0, s2mm_bohdl[i]);
		rval = xrtRunSetArg(s2mm_rhdl[i], 2, NUM_CUBOIDS*NUM_OUTPUT_SAMPLES);
	    }

	    printf ("Run time args passed to s2mm DMAs\n");
	}


	void allocBuffers(xrtDeviceHandle dhdl)
	{
	    //Allocate BOs (buffer objects) of requested size with appropriate flags
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		s2mm_bohdl[i] = xrtBOAlloc(dhdl, sizeof(cfloat)*NUM_OUTPUT_SAMPLES*NUM_CUBOIDS, 0, 0);
	    }

	    //printf ("Allocated Buffer handles to mm2s and s2mm\n");

	    //Memory map BOs into user's address space (DDR Memory)
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		s2mm_mapped[i] = reinterpret_cast<cfloat*>(xrtBOMap(s2mm_bohdl[i]));
	    }
	}

#ifndef MATLAB_TCP_MULTI_CUBE
	cfloat* get_weight_golden_output(int i) 
	{
	    switch (i) {
		case 0 : 
		    return weightAppOut0_data;
		case 1 : 
		    return weightAppOut1_data;
		case 2 : 
		    return weightAppOut2_data;
		case 3 : 
		    return weightAppOut3_data;
		default:
		    assert(false);
	    }
	    return 0;
	}

	void checkResult() {
	    int matchCnt = 0;
	    //printf ("Doing DI Check\n");
	    // Compare 1000 weight outputs written to each of the 4 s2mm DMAs 
	    for(unsigned int j = 0; j < R_DATAMOVER_INSTS; j++) {
		cfloat* gout = get_weight_golden_output(j);
		for(unsigned int i = 0; i < NUM_CUBOIDS*NUM_OUTPUT_SAMPLES; i++) {
		    if (compare(s2mm_mapped[j][i].real, gout[i%NUM_OUTPUT_SAMPLES].real)) {
			std::cout << "Error in s2mm DMA 0 REAL : COUT: "<< i << " : " 
			              << std::hex << s2mm_mapped[0][i].real  <<  " :  " << std::hex<< weightAppOut0_data[i].real << std::endl;
			errCnt++;
		    } else {
			matchCnt++;
			////std::cout << "s2mm DMA 0 REAL COUT: "<< i << " : " 
			//          << std::hex << s2mm_mapped[0][i].real  <<  " :  " << std::hex<< weightAppOut0_data[i].real << std::endl;
		    }
		    if (compare(s2mm_mapped[j][i].imag, gout[i%NUM_OUTPUT_SAMPLES].imag)){
			//std::cout << "Error in s2mm DMA 4 REAL : COUT: "<< i << " : " << std::hex << s2mm_mapped[4][i].real  <<  " :  " << std::hex<< weightAppOut4_data[i].real << std::endl;
			errCnt++;
		    } else {
			matchCnt++;
			//std::cout << "s2mm DMA 4 REAL COUT: "<< i << " : " << std::hex << s2mm_mapped[4][i].real  <<  " :  " << std::hex<< weightAppOut4_data[i].real << std::endl;
		    }
		}
	    }
	    printf("\nMatch count = %d\n", matchCnt);
	    std::cout << "TEST " << (errCnt ? "FAILED " : "PASSED") << std::endl;
	}
#else
	void sendResult(serial_processor &sp)
    {
		/*std::ofstream ofile;
		ofile.open("output.txt",std::ios::out);
		for(unsigned int k = 0; k <  NUM_CUBOIDS; k++) {
		for(unsigned int j = 0; j < R_DATAMOVER_INSTS; j++) {
		for(unsigned int i = 0; i < NUM_OUTPUT_SAMPLES; i++) {
			ofile << s2mm_mapped[j][i+k*NUM_OUTPUT_SAMPLES].real << " " << s2mm_mapped[j][i+k*NUM_OUTPUT_SAMPLES].imag <<std::endl;
		}
	    }
		}
		printf("\nS2MM DATA DUMP COMPLETE\n"); */
	    for(unsigned int i = 0; i < NUM_CUBOIDS; i++)
        {
            for(unsigned int j = 0; j < R_DATAMOVER_INSTS; j++)
            {
                memcpy(&result[j][0], &s2mm_mapped[j][i*NUM_OUTPUT_SAMPLES], NUM_OUTPUT_SAMPLES * sizeof(cfloat));
            }

            /* 
             * Send 4x1000 cfloat result for current cuboid
             * Matlab sequence number counts 1->10 rather than 0->9, so add 1 to curr_cuboid
             */
            sp.tx_result(result, curr_cuboid+1);

            //printf("Cube Result:\n");
            //mem_dump((uint8_t *)result, 100);
        }
    }
#endif

	void clear() {
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		xrtBOFree(s2mm_bohdl[i]);
	    }
	}

	void run() {

	    // Execute the Data Mover Kernels 
	    //Create a kernel run handle to start each data mover pl kernel
	    //mm2s kernels moves data from DDR to AI Engine
	    //s2mm kernel moves data from AI Engine to DDR 
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		xrtRunStart(s2mm_rhdl[i]);
	    }

	    printf ("s2mm running\n");
	}

	void close(void)
	{
	    //printf ("Close mm2s/s2mm \n");

	    //Wait for hls DMA execution to finish
	    //Close the run handles obtained by in xrtKernelRun()  
	    //Close opened kernel handles obtained from xrtPLKernelOpen()
	    for (int i = 0; i < R_DATAMOVER_INSTS; ++i) {
		xrtRunClose(s2mm_rhdl[i]);
		xrtKernelClose(s2mm_khdl[i]);
	    }

	}

	void waitTo_complete(void)
	{
	    while(
		    ERT_CMD_STATE_COMPLETED != xrtRunState(s2mm_rhdl[0]) ||
		    ERT_CMD_STATE_COMPLETED != xrtRunState(s2mm_rhdl[1]) ||
		    ERT_CMD_STATE_COMPLETED != xrtRunState(s2mm_rhdl[2]) ||
		    ERT_CMD_STATE_COMPLETED != xrtRunState(s2mm_rhdl[3]));
	    printf ("S2MM COMPLETED\n");
	}

	S2MM()
	    : errCnt(0)
	{}

};

class CHSelector
{
    private:
	xrtKernelHandle chs_khdl[CH_SELECTOR_INSTS];
	xrtRunHandle chs_rhdl[CH_SELECTOR_INSTS];

    public:
	void open(xrtDeviceHandle dhdl, const axlf *top)
	{

	    for (int i = 0; i < CH_SELECTOR_INSTS; ++i) {
		std::stringstream s;
		s << "chanSelector_wrapper:{chanSelector_wrapper_" << i << "}"; 
		chs_khdl[i] = xrtPLKernelOpen(dhdl, top->m_header.uuid, s.str().c_str());
		s.str("");
	    }
	    for (int i = 0; i < CH_SELECTOR_INSTS; ++i) {
		chs_rhdl[i] = xrtRunOpen(chs_khdl[i]);
	    }
	}

	void setArgs(xrtDeviceHandle dhdl, const axlf *top)
	{
	    //Set channel select argument
	    int rval;
	    for (int i = 0; i < CH_SELECTOR_INSTS; ++i) {
		rval = xrtRunSetArg(chs_rhdl[i], 2, 6);
		rval = xrtRunSetArg(chs_rhdl[i], 4, NUM_CUBOIDS*NUM_INPUT_SAMPLES);
	    }
	    printf ("Run time args passed to CH Selector\n");
	}


	void run() {

	    // Execute the Data Mover Kernels 
	    //Create a kernel run handle to start each data mover pl kernel
	    for (int i = 0; i < CH_SELECTOR_INSTS; ++i) {
		xrtRunStart(chs_rhdl[i]);
	    }

	    printf ("CH Selector running\n");
	}

	void close(void)
	{
	    //printf ("Close CH Selector \n");

	    //Wait for hls DMA execution to finish
	    //Close the run handles obtained by in xrtKernelRun()  
	    //Close opened kernel handles obtained from xrtPLKernelOpen()
	    for (int i = 0; i < CH_SELECTOR_INSTS; ++i) {
		xrtRunClose(chs_rhdl[i]);
		xrtKernelClose(chs_khdl[i]);
	    }
	}

	void waitTo_complete(void)
	{
	    while((ERT_CMD_STATE_COMPLETED != xrtRunState(chs_rhdl[0])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(chs_rhdl[1])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(chs_rhdl[2])) ||
		    (ERT_CMD_STATE_COMPLETED != xrtRunState(chs_rhdl[3])));
	    printf ("CH Selector completed\n");
	}

};


xrtGraphHandle graphHandle;
S2MM s2mmInst;
MM2S mm2sInst;
CHSelector chsInst;
DataBufferStore dataBufferStore[SHARED_BUFFERS];
CfgBufferStore wCfgBufferStore[W_CFG_COUNT];
CfgBufferStore rCfgBufferStore[R_CFG_COUNT];
wdma wDatamover;
rdma rDatamover;
std::thread wdmas;
std::thread rdmas;
xrtBufferHandle ping_parent_data_bo_hdl;
xrtBufferHandle pong_parent_data_bo_hdl;

#if 0
bool ddr_is_ready(int i /* 0 ping | 1 pong */, int c) {

    if (i == PING_BUFFER_FLAG) {
	return cmpl_wdma_ping_count.load() == 1 * (c+1) &&
	    cmpl_rdma_pong_count.load() % 1  == 0;
    } else if (i == PONG_BUFFER_FLAG) {
	return  cmpl_wdma_pong_count.load()  == 1 * (c+1) &&
	    cmpl_rdma_ping_count.load() % 1 == 0;
    } else {
	assert(!"Unsupported id");
    }
    return false;
}

bool ddr_is_free(int i /* 0 ping | 1 pong */, int c) {

    if (i == PING_BUFFER_FLAG) {
	return cmpl_rdma_ping_count.load() == 1 * c &&
	    cmpl_wdma_pong_count.load() % 1  == 0;
    } else if (i == PONG_BUFFER_FLAG) {
	return cmpl_rdma_pong_count.load() == 1 * c && 
	    cmpl_wdma_ping_count.load() % 1  == 0;
    } else {
	assert(!"Unsupported id");
    }

    return false;
}
#endif

void initGraph(xrtDeviceHandle dhdl, const axlf* top)
{
    graphHandle = xrtGraphOpen(dhdl, (unsigned char*) top->m_header.uuid, "mygraph");
    if (!graphHandle)
	throw std::runtime_error("Unable to open graph handle");
}
#ifndef MATLAB_TCP_MULTI_CUBE
void runGraph()
{
    int ret;
    ret = xrtGraphReset(graphHandle);
    if (ret)
	throw std::runtime_error("Unable to reset graph");

        ret = xrtGraphRun(graphHandle, NUM_CUBOIDS);
        if (ret)
    	   throw std::runtime_error("Unable to run graph");

	for(int i=0; i<NUM_CUBOIDS; i++) {
        	//printf("Updating RTPs for iteration %d\n",i+1);
        	xrtGraphUpdateRTP(graphHandle, "mygraph.ksteer.in[0]", (char*)&NORMALIZED_DOPPLER , sizeof(float));
        	xrtGraphUpdateRTP(graphHandle, "mygraph.ksteer.in[1]", (char*)&NORMALIZED_SPATIAL , sizeof(float));
	}

//    for(int i=0; i<NUM_CUBOIDS; i++) {
//	printf("Waiting for graph iteration %d to finish\n",i+1);
//        xrtGraphWait(graphHandle,0);
//	printf("Finisehd graph iteration %d\n",i+1);
//    }
}	
#else
void runGraph(cube_params_t cube_params[NUM_CUBOIDS])
{
    int ret;
    ret = xrtGraphReset(graphHandle);
    if (ret)
	throw std::runtime_error("Unable to reset graph");

    //printf("Updating RTPs for graph\n");
    xrtGraphUpdateRTP(graphHandle, "mygraph.ksteer.in[0]", (char*)&cube_params[curr_cuboid].norm_dop_freq, sizeof(float));
    xrtGraphUpdateRTP(graphHandle, "mygraph.ksteer.in[1]", (char*)&cube_params[curr_cuboid].norm_spa_freq, sizeof(float));
	//printf("Updated RTPs for graph\n");

//    for(int i=0; i<NUM_CUBOIDS; i++) {
        ret = xrtGraphRun(graphHandle, NUM_CUBOIDS);
		//printf("Running graph\n");
        if (ret)
    	   throw std::runtime_error("Unable to run graph");

//	printf("Waiting for graph iteration %d to finish\n",i+1);
//        xrtGraphWait(graphHandle,0);
//	printf("Finisehd graph iteration %d\n",i+1);
//    }
}	
#endif
void closeGraph() 
{
    xrtGraphClose(graphHandle);
}

    std::vector<char>
load_xclbin(xrtDeviceHandle device, const std::string& fnm)
{
    // load bit stream
    std::ifstream stream(fnm);
    if (! stream.good())
	throw std::runtime_error("Speified file does not exist or has no read permission.");

    stream.seekg(0,stream.end);
    size_t size = stream.tellg();
    stream.seekg(0,stream.beg);

    std::vector<char> header(size);
    stream.read(header.data(),size);

    const axlf* top = reinterpret_cast<const axlf*>(header.data());
    if (xrtDeviceLoadXclbin(device, top))
	throw std::runtime_error("Bitstream download failed");

    return header;
}

void initAndRunWDMA(wdma* wDatamover, xrtDeviceHandle dhdl,
	const axlf *top, int i, DataBufferStore* dataBufferStoreOne,
	DataBufferStore* dataBufferStoreTwo, CfgBufferStore* cfgBufferStore) {

    int current_buffer = 0;
    int count = 0;
//    while (count != NUM_CUBOIDS) {
//	if (0 == current_buffer) {
//	    if (ddr_is_free(PING_BUFFER_FLAG, count / 2)) {
//		//printf("WDMA PING %d : START %d\n ", i, count );
		wDatamover->init(dhdl, top, i, dataBufferStoreOne, dataBufferStoreTwo, *cfgBufferStore);
#ifndef MATLAB_TCP_MULTI_CUBE
        wdma_start = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> dma_iter_start = wdma_start - start;
        //std::cout << "WDMA " << i << "cuboid" << count << " start time : " << dma_iter_start.count() << std::endl;
#endif
		wDatamover->run();

}


void initAndRunRDMA(rdma* rDatamover, xrtDeviceHandle dhdl,
	const axlf *top, int i, xrtBufferHandle* ping_parent_hdl,
	xrtBufferHandle* pong_parent_hdl, CfgBufferStore* cfgBufferStore) {

    int current_buffer = 0;
    int count = 0;
//    while (count != NUM_CUBOIDS) {
//	if (0 == current_buffer) {
//	    if (ddr_is_ready(PING_BUFFER_FLAG, count / 2)) {
//		//printf("RDMA PING %d : START %d \n", i, count );
		rDatamover->init(dhdl, top, i, *ping_parent_hdl, *pong_parent_hdl, *cfgBufferStore);
#ifndef MATLAB_TCP_MULTI_CUBE
        rdma_start = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> dma_iter_start = rdma_start - start;
        //std::cout << "RDMA " << i << "cuboid" << count << " start time : " << dma_iter_start.count() << std::endl;
#endif
		rDatamover->run();
}

void allocDataBuffers(xrtDeviceHandle dhdl, const axlf* top)
{
    //////////////////////////////////////////
    // Global Parent Buffers Init
    //////////////////////////////////////////
    uint64_t *ping_data_bo_mapped;
    size_t data_size_in_bytes = N * sizeof(uint64_t);
    ping_parent_data_bo_hdl = xrtBOAlloc(dhdl, data_size_in_bytes, 0, group_id);
    ping_data_bo_mapped = reinterpret_cast<uint64_t *>(xrtBOMap(ping_parent_data_bo_hdl));

    memset(ping_data_bo_mapped, 0, data_size_in_bytes);
    //std::cout << "\n\nParent Ping Data buffer virtual addr " << ping_data_bo_mapped << ".\n\n" << std::endl;

    uint64_t *pong_data_bo_mapped;
    pong_parent_data_bo_hdl = xrtBOAlloc(dhdl, data_size_in_bytes, 0, group_id);
    pong_data_bo_mapped = reinterpret_cast<uint64_t *>(xrtBOMap(pong_parent_data_bo_hdl));

    memset(pong_data_bo_mapped, 0, data_size_in_bytes);
    //std::cout << "\n\nParent Pong Data buffer virtual addr " << pong_data_bo_mapped << ".\n\n" << std::endl;


    size_t offset = 0; 
    for(int i = 0; i < SHARED_BUFFERS/2; ++i)
    {
	//printf("Initialising child ping data buffer store %d...\n", i);
	dataBufferStore[i].initData(ping_parent_data_bo_hdl, offset);

	//offset = offset + data_size_in_bytes/4;
    }
    offset = 0; // Setting start to 0
    for(int i = SHARED_BUFFERS/2; i < SHARED_BUFFERS; ++i)
    {
	//printf("Initialising child pong data buffer store %d...\n", i);
	dataBufferStore[i].initData(pong_parent_data_bo_hdl, offset);

	//offset = offset + data_size_in_bytes/4;
    }

}

void allocCFGBuffers(xrtDeviceHandle dhdl, const axlf* top)
{
    uint64_t w_config_arr0[] = {1, 2000, 8000, 10, 0, 1, 0, 1};
    //printf("Initialising write cfg buffer store %d...\n", 0);
    wCfgBufferStore[0].initCFG(dhdl, w_config_arr0);
    uint64_t w_config_arr1[] = {2000, 1, 2000, 8000, 10, 0, 1, 0, 1};
    //printf("Initialising write cfg buffer store %d...\n", 1);
    wCfgBufferStore[1].initCFG(dhdl, w_config_arr1);
    uint64_t w_config_arr2[] = {4000, 1, 2000, 8000, 10, 0, 1, 0, 1};
    //printf("Initialising write cfg buffer store %d...\n", 2);
    wCfgBufferStore[2].initCFG(dhdl, w_config_arr2);
    uint64_t w_config_arr3[] = {6000, 1, 2000, 8000, 10, 0, 1, 0, 1};
    //printf("Initialising write cfg buffer store %d...\n", 3);
    wCfgBufferStore[3].initCFG(dhdl, w_config_arr3);

    uint64_t r_config_arr[] = {1, 125, 2000, 4, 8000, 10, 125, 4};
    for(int i = 0; i < R_CFG_COUNT; ++i)
    {
	//r_config_arr[0] = i * 250;
	//printf("Initialising read cfg buffer store %d...\n", i);
	rCfgBufferStore[i].initCFG(dhdl, r_config_arr);
    }

}

void runDMAs(xrtDeviceHandle dhdl, const axlf* top)
{
    //std::cout << "\nRun wdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	//wdmas = std::thread(initAndRunWDMA, &wDatamover, dhdl, top, i, &dataBufferStore[i], 
	//	&dataBufferStore[i + SHARED_BUFFERS / 2], &wCfgBufferStore[i]);
	initAndRunWDMA(&wDatamover, dhdl, top, i, &dataBufferStore[i], 
		&dataBufferStore[i + SHARED_BUFFERS / 2], &wCfgBufferStore[i]);
    }

    //std::cout << "\nRun rdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	//rdmas = std::thread(initAndRunRDMA, &rDatamover, dhdl, top, i, &ping_parent_data_bo_hdl,
	//	&pong_parent_data_bo_hdl, &rCfgBufferStore[i]);
	initAndRunRDMA(&rDatamover, dhdl, top, i, &ping_parent_data_bo_hdl,
		&pong_parent_data_bo_hdl, &rCfgBufferStore[i]);
    }

}

#ifndef MATLAB_TCP_MULTI_CUBE
void waitDMAs(int i)
{
		wDatamover.waitTo_complete(PING_BUFFER_FLAG);
        wdma_end = std::chrono::high_resolution_clock::now();
        //std::chrono::duration<double> dma_iter_end = wdma_end - start;
        //std::cout << "WDMA " << "cuboid" << " end time : " << dma_iter_end.count() << std::endl;
        wdma_time[i] = wdma_end - wdma_start;
        //std::cout << "WDMA " << "cuboid" << " time : " << dma_iter_end.count() << std::endl;
		rDatamover.waitTo_complete(PING_BUFFER_FLAG);
        rdma_end = std::chrono::high_resolution_clock::now();
        //dma_iter_end = rdma_end - start;
        //std::cout << "RDMA " << "cuboid" << " end time : " << dma_iter_end.count() << std::endl;
        rdma_time[i] = rdma_end - rdma_start;
        //std::cout << "RDMA " << "cuboid" << " time : " << dma_iter_end.count() << std::endl;

}
#else
void waitDMAs()
{
	wDatamover.waitTo_complete(PING_BUFFER_FLAG);
	rDatamover.waitTo_complete(PING_BUFFER_FLAG);
}
#endif

void openDMAs(xrtDeviceHandle dhdl, const axlf* top)
{
    //Opening PL-Handles
    //std::cout << "\nOpening wdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	wDatamover.open(dhdl, top, i);
    }

    //std::cout << "\nOpening rdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	rDatamover.open(dhdl, top, i);
    }

}

void closeDMAs()
{
    //Closing PL-Handles
    //std::cout << "\nClosing wdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	wDatamover.close();
    }

    //std::cout << "\nClosing rdma handles...\n";
    for(int i = 0; i < 1; ++i)
    {
	rDatamover.close();
    }
}

void initDMAs(xrtDeviceHandle dhdl, const axlf* top)
{
    // Initialize WDMA/RDMA kernels
	openDMAs(dhdl, top);
    allocDataBuffers(dhdl, top);	
    allocCFGBuffers(dhdl, top);
} 

void checkArguments(int argc, char ** argv) {
    if(argc < 2) {
	std::cout << "Usage: " << argv[0] <<" <xclbin>" << std::endl;
	exit(1);
    }
}

void resetCounters() 
{
    cmpl_rdma_ping_count = 0;
    cmpl_rdma_pong_count = 0;

    cmpl_wdma_ping_count = 0;
    cmpl_wdma_pong_count = 0;

}

#ifndef MATLAB_TCP_MULTI_CUBE
void initJESD(xrtDeviceHandle dhdl, const axlf* top)
{
    // Instantiate jesd_platform
    s2mmInst.open(dhdl, top);
    s2mmInst.allocBuffers(dhdl);
    mm2sInst.open(dhdl, top);
	chsInst.open(dhdl,top);
    mm2sInst.allocBuffers(dhdl);
    mm2sInst.writeData();

    printf ("Initializing tc_selector registers\n");
	// Initialize tc_selector register configuration
    system("devmem 0xA4170004 32 0xa0fa0");
    system("devmem 0xA4170008 32 0x00004");
    system("devmem 0xA417000c 32 0x007d0");
    system("devmem 0xA4170010 32 0x00546");
    system("devmem 0xA4170014 32 0x00004");
    system("devmem 0xA4170018 32 0x007d0");
    system("devmem 0xA417001c 32 0x00546");
    system("devmem 0xA4170020 32 0x00004");
    system("devmem 0xA4170024 32 0x007d0");
    system("devmem 0xA4170028 32 0x00546");
    system("devmem 0xA417002c 32 0x00004");
    system("devmem 0xA4170030 32 0x007d0");
    system("devmem 0xA4170034 32 0x00546");
    system("devmem 0xA4170038 32 0x00004");
    system("devmem 0xA417003c 32 0x007d0");
    system("devmem 0xA4170040 32 0x00546");
    system("devmem 0xA4170044 32 0x00004");
    system("devmem 0xA4170048 32 0x007d0");
    system("devmem 0xA417004c 32 0x00546");
    system("devmem 0xA4170050 32 0x00004");
    system("devmem 0xA4170054 32 0x007d0");
    system("devmem 0xA4170058 32 0x00546");
    system("devmem 0xA417005c 32 0x00004");
    system("devmem 0xA4170060 32 0x007d0");
    system("devmem 0xA4170064 32 0x00546");
    system("devmem 0xA4170068 32 0x00004");
    system("devmem 0xA417006c 32 0x007d0");
    system("devmem 0xA4170070 32 0x00546");
    system("devmem 0xA4170074 32 0x00004");
    system("devmem 0xA4170078 32 0x007d0");
    system("devmem 0xA417007c 32 0x00546");
	system("devmem 0xA4130000 32 0x00006");
	system("devmem 0xA4140000 32 0x00006");
	system("devmem 0xA4150000 32 0x00006");
	system("devmem 0xA4160000 32 0x00006");

}
#else
void initJESD(xrtDeviceHandle dhdl, const axlf* top, uint32_t *cube_data, cube_params_t cube_params[NUM_CUBOIDS])
{
    // Instantiate jesd_platform
    s2mmInst.open(dhdl, top);
    s2mmInst.allocBuffers(dhdl);
    mm2sInst.open(dhdl, top);
	chsInst.open(dhdl,top);
    mm2sInst.allocBuffers(dhdl);
    //mm2sInst.writeData(cube_data);

    // Comment, these, and replace with arrays from Matlab
#if 0
    unsigned int gc[10] = {4,4,4,4,4,4,4,4,4,4};
    unsigned int tc[10] = {2000,2000,2000,2000,2000,2000,2000,2000,2000,2000};
    unsigned int trangeIdx[10] = {1350,1350,1350,1350,1350,1350,1350,1350,1350,1350};
#endif
    
    system("devmem 0xA4170004 32 0xa0fa0");

    string a,b,c;
    unsigned int base_addr=0xA4170000;
    unsigned int addr=base_addr+8;
    const char *command;

    for(int i=0;i<10;i++) {
	a="";
	b="";
	c="";
	a=to_string(addr);
	b=to_string(cube_params[i].gc);
	c="devmem " + a + " 32 " + b;
	command=c.c_str();
	system(command);
	addr = addr+4;

	a=to_string(addr);
	b=to_string(cube_params[i].tc);
	c="devmem " + a + " 32 " + b;
	command=c.c_str();
	system(command);
	addr = addr+4;

	a=to_string(addr);
	b=to_string(cube_params[i].trangeIdx);
	c="devmem " + a + " 32 " + b;
	command=c.c_str();
	system(command);
	addr = addr+4;
    }

	system("devmem 0xA4130000 32 0x00006");
	system("devmem 0xA4140000 32 0x00006");
	system("devmem 0xA4150000 32 0x00006");
	system("devmem 0xA4160000 32 0x00006");
} 
#endif

void closeJESD()
{
    s2mmInst.close();
    mm2sInst.close();
	chsInst.close();
}

void runMM2S(xrtDeviceHandle dhdl, const axlf* top) 
{
    int i = 0;
    while (INFINITE || i != NUM_ITERATIONS) {
	mm2sInst.setArgs(dhdl, top);
#ifndef MATLAB_TCP_MULTI_CUBE
	auto mm2s_start = std::chrono::high_resolution_clock::now();
#endif
	mm2sInst.run();
	mm2sInst.waitTo_complete();
#ifndef MATLAB_TCP_MULTI_CUBE
	auto mm2s_end = std::chrono::high_resolution_clock::now(); 
	std::chrono::duration<double> mm2s_iter_start = mm2s_start - start;
	std::cout << "mm2s iteration " << i << " start time : " << mm2s_iter_start.count() << std::endl;
	std::chrono::duration<double> mm2s_iter_end = mm2s_end - start;
	std::cout << "mm2s iteration " << i << " end time : " << mm2s_iter_start.count() << std::endl;
#endif
	++i;
    }
}

#ifndef MATLAB_TCP_MULTI_CUBE
void runJESD(xrtDeviceHandle dhdl, const axlf* top) 
{
    int i = 0;
	std::cout << "\nSetting Arguments for mm2s...\n";
	mm2sInst.setArgs(dhdl, top);
	std::cout << "\nSetting Arguments for s2mm...\n";
	//s2mmInst.setArgs(dhdl, top);
	std::cout << "\nSetting Arguments for CH Selector...\n";
	chsInst.setArgs(dhdl, top);
    //auto start = std::chrono::high_resolution_clock::now();
    while (INFINITE || i != NUM_ITERATIONS) {
	start = std::chrono::high_resolution_clock::now();
	//runGraph();
	chsInst.run();
	mm2sInst.run();
	s2mmInst.setArgs(dhdl, top);
	s2mmInst.run();
	//chsInst.run();
        //auto dma_start = std::chrono::high_resolution_clock::now();
	runDMAs(dhdl, top);
	runGraph();
	mm2sInst.waitTo_complete();
	chsInst.waitTo_complete();
	waitDMAs(i);
	//auto dma_end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> dma_diff = dma_end - dma_start;
	//std::cout << "DMA Completed " << i << " iteration, overall duration: " << dma_diff.count() << std::endl;
	s2mmInst.waitTo_complete();
	// Commenting out data integrity check for demo version
	// s2mm_mapped[0-15] arrays each hold 250 weightApp output samples which need to be sent back to Matlab
	s2mmInst.checkResult();
	//resetCounters();
	//auto end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> diff = end - start;
	//std::cout << "Completed " << i << " iteration, duration: " << diff.count() << std::endl;
	++i;
    }
}
#else
void runJESD(serial_processor &sp, xrtDeviceHandle dhdl, const axlf* top, uint32_t *cube_data, cube_params_t cube_params[NUM_CUBOIDS]) 
{
    int i = 0;
    //auto start = std::chrono::high_resolution_clock::now();
    while (INFINITE || i != NUM_ITERATIONS) {
    mm2sInst.setArgs(dhdl, top);
    mm2sInst.writeData(cube_data);
	//s2mmInst.setArgs(dhdl, top);
	chsInst.setArgs(dhdl, top);
	chsInst.run();
	mm2sInst.run();
	s2mmInst.setArgs(dhdl, top);
	s2mmInst.run();
	
	//if(i%2 == 0)
	runDMAs(dhdl, top);
	runGraph(cube_params);
	mm2sInst.waitTo_complete();
	chsInst.waitTo_complete();
	//if(i%2 == 1)
	waitDMAs();
	//mm2sInst.waitTo_complete();
	//chsInst.waitTo_complete();
	s2mmInst.waitTo_complete();
	// Commenting out data integrity check for demo version
	// s2mm_mapped[0-3] arrays each hold 1000 weightApp output samples which need to be sent back to Matlab
	//s2mmInst.checkResult();
	s2mmInst.sendResult(sp);
	resetCounters();
	//auto end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> diff = end - start;
	//std::cout << "Completed " << i << " iteration, overall duration: " << diff.count() << std::endl;
	++i;
	if (curr_cuboid==9)
		curr_cuboid = 0;
	else
		++curr_cuboid;
	// Sleep some time to allow Matlab to generate plots before
    // Note: We may need to run sendResult() from a dedicated thread to avoid sleeping in the main loop 
	sleep(10);
    }
}
#endif

#ifndef MATLAB_TCP_MULTI_CUBE
void runDesign(xrtDeviceHandle dhdl, const axlf* top) 
{
    //std::thread mm2sThrd = std::thread(runMM2S, dhdl, top);
    runJESD(dhdl, top);

	for(int i=0; i<NUM_ITERATIONS; i++) {
		std::cout << "WDMA " << i << " iteration duration: " << wdma_time[i].count() << ".\n" << std::endl;
	}
	for(int i=0; i<NUM_ITERATIONS; i++) {
		std::cout << "RDMA " << i << " iteration duration: " << rdma_time[i].count() << ".\n" << std::endl;
	}
    //mm2sThrd.join();
}
#else
void runDesign(serial_processor &sp, xrtDeviceHandle dhdl, const axlf* top, uint32_t *cube_data, cube_params_t cube_params[NUM_CUBOIDS])
{
    runJESD(sp, dhdl, top, cube_data, cube_params);
}
#endif

void clearBuffers()
{
    s2mmInst.clear();
    mm2sInst.clear();
    // Clear XRT BO buffers
    for(int i = 0; i < W_CFG_COUNT; ++i)
    {
	wCfgBufferStore[i].clear();
    }

    for(int i = 0; i < R_CFG_COUNT; ++i)
    {
	rCfgBufferStore[i].clear();
    }

    for(int i = 0; i < SHARED_BUFFERS; ++i)
    {
	dataBufferStore[i].clear();
    }

}

#ifndef MATLAB_TCP_MULTI_CUBE
int main(int argc, char ** argv) {
    start = std::chrono::high_resolution_clock::now();
    checkArguments(argc, argv);

    auto dhdl = xrtDeviceOpen(0);
    std::vector<char> header = load_xclbin(dhdl, argv[1]);
    auto top = reinterpret_cast<const axlf*>(header.data());
    initGraph(dhdl, top);
    initJESD(dhdl, top);
    initDMAs(dhdl, top);

    runDesign(dhdl, top);

    closeJESD();
    closeDMAs();
    closeGraph();
    clearBuffers();

    xrtDeviceClose(dhdl);
    return 0;
}
#else
int main(int argc, char** argv) 
{
    int opt;
    unsigned short port = 0;

    boost::asio::io_context io_context;

    while ((opt = getopt(argc, argv, "p:f:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                xclbinFilename = optarg;
                break;
            default:
                break;
        }
    }

    if (port != 0 && xclbinFilename != NULL)
    {
        printf("port is:  %d\n", port);
        printf("fname is: %s\n", xclbinFilename);

        Server server(io_context, port);
        server.start();
        io_context.run();
    }
    else
    {
        printf("usage: %s -p <port_number> -f <xclbin>\n", argv[0]);
    }

    return 0;
}

int run_adpca(serial_processor &sp, 
              uint32_t *cube_data, size_t cube_data_size,
              cube_params_t cube_params[NUM_CUBOIDS])
{
    auto dhdl = xrtDeviceOpen(0);
    std::vector<char> header = load_xclbin(dhdl, xclbinFilename);
    auto top = reinterpret_cast<const axlf*>(header.data());
    initGraph(dhdl, top);
    initJESD(dhdl, top, cube_data, cube_params);
    initDMAs(dhdl, top);

    runDesign(sp, dhdl, top, cube_data, cube_params);

    closeJESD();
    closeDMAs();
    closeGraph();
    clearBuffers();

    xrtDeviceClose(dhdl);
    return 0;
}
#endif
