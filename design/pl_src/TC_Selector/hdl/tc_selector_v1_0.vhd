--
--Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
--SPDX-License-Identifier: MIT
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tc_selector_v1_0 is
	generic (
		-- Users to add parameters here
		C_NB_PULSES             : integer   := 10;
		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 7
	);
	port (
		-- Users to add ports here
		
		-- Global reset
		
		
        -- Antenna AXI Stream Interfaces
        ant_axis_aresetn        : in std_logic;
        ant_axis_aclk           : in std_logic;
        
        ant0_axis_tdata         : in std_logic_vector(63 downto 0);
        ant0_axis_tvalid        : in std_logic;
        ant0_axis_tready        : out std_logic;
 
        ant1_axis_tdata         : in std_logic_vector(63 downto 0);
        ant1_axis_tvalid        : in std_logic;
        ant1_axis_tready        : out std_logic;
                
        ant2_axis_tdata         : in std_logic_vector(63 downto 0);
        ant2_axis_tvalid        : in std_logic;
        ant2_axis_tready        : out std_logic;
        
        ant3_axis_tdata         : in std_logic_vector(63 downto 0);
        ant3_axis_tvalid        : in std_logic;
        ant3_axis_tready        : out std_logic;
        
        ant_fifo_ovfl           : out std_logic;
        ant_fifo_unfl           : out std_logic;

        -- Internal Pulse FIFOs flags
        prev_pulse_fifo_ovfl    : out std_logic;
        prev_pulse_fifo_unfl    : out std_logic;
        curr_pulse_fifo_ovfl    : out std_logic;
        curr_pulse_fifo_unfl    : out std_logic;
        
        -- AIE Covariance Kernels Interfaces 
        cov_axis_aclk           : in std_logic;
        cov_axis_aresetn        : in std_logic;

        cov0_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov0_prev_axis_tvalid   : out std_logic;
        cov0_prev_axis_tready   : in std_logic;
        
        cov0_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov0_curr_axis_tvalid   : out std_logic;
        cov0_curr_axis_tready   : in std_logic;
        
        cov1_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov1_prev_axis_tvalid   : out std_logic;
        cov1_prev_axis_tready   : in std_logic;
        
        cov1_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov1_curr_axis_tvalid   : out std_logic;
        cov1_curr_axis_tready   : in std_logic;       
        
        cov2_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov2_prev_axis_tvalid   : out std_logic;
        cov2_prev_axis_tready   : in std_logic;
        
        cov2_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov2_curr_axis_tvalid   : out std_logic;
        cov2_curr_axis_tready   : in std_logic;
        
        cov3_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov3_prev_axis_tvalid   : out std_logic;
        cov3_prev_axis_tready   : in std_logic;
        
        cov3_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov3_curr_axis_tvalid   : out std_logic;
        cov3_curr_axis_tready   : in std_logic;
        
        cov4_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov4_prev_axis_tvalid   : out std_logic;
        cov4_prev_axis_tready   : in std_logic;
        
        cov4_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov4_curr_axis_tvalid   : out std_logic;
        cov4_curr_axis_tready   : in std_logic;
        
        cov5_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov5_prev_axis_tvalid   : out std_logic;
        cov5_prev_axis_tready   : in std_logic;
              
        cov5_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov5_curr_axis_tvalid   : out std_logic;
        cov5_curr_axis_tready   : in std_logic;
           
        cov6_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov6_prev_axis_tvalid   : out std_logic;
        cov6_prev_axis_tready   : in std_logic;
         
        cov6_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov6_curr_axis_tvalid   : out std_logic;
        cov6_curr_axis_tready   : in std_logic;

        cov7_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov7_prev_axis_tvalid   : out std_logic;
        cov7_prev_axis_tready   : in std_logic;
                       
        cov7_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov7_curr_axis_tvalid   : out std_logic;
        cov7_curr_axis_tready   : in std_logic;

        cov8_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov8_prev_axis_tvalid   : out std_logic;
        cov8_prev_axis_tready   : in std_logic;
        
        cov8_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov8_curr_axis_tvalid   : out std_logic;
        cov8_curr_axis_tready   : in std_logic;
        
        cov9_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov9_prev_axis_tvalid   : out std_logic;
        cov9_prev_axis_tready   : in std_logic;
        
        cov9_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov9_curr_axis_tvalid   : out std_logic;
        cov9_curr_axis_tready   : in std_logic;
        
        cov10_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov10_prev_axis_tvalid  : out std_logic;
        cov10_prev_axis_tready  : in std_logic;
        
        cov10_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov10_curr_axis_tvalid  : out std_logic;
        cov10_curr_axis_tready  : in std_logic;
          
        cov11_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov11_prev_axis_tvalid  : out std_logic;
        cov11_prev_axis_tready  : in std_logic;
        
        cov11_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov11_curr_axis_tvalid  : out std_logic;
        cov11_curr_axis_tready  : in std_logic;
        
        cov12_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov12_prev_axis_tvalid  : out std_logic;
        cov12_prev_axis_tready  : in std_logic;
        
        cov12_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov12_curr_axis_tvalid  : out std_logic;
        cov12_curr_axis_tready  : in std_logic;
        
        cov13_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov13_prev_axis_tvalid  : out std_logic;
        cov13_prev_axis_tready  : in std_logic;
        
        cov13_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov13_curr_axis_tvalid  : out std_logic;
        cov13_curr_axis_tready  : in std_logic;
                
        cov14_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov14_prev_axis_tvalid  : out std_logic;
        cov14_prev_axis_tready  : in std_logic;
        
        cov14_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov14_curr_axis_tvalid  : out std_logic;
        cov14_curr_axis_tready  : in std_logic;
        
        cov15_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov15_prev_axis_tvalid  : out std_logic;
        cov15_prev_axis_tready  : in std_logic;
        
        cov15_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov15_curr_axis_tvalid  : out std_logic;
        cov15_curr_axis_tready  : in std_logic;
        
		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in std_logic;
		s00_axi_aresetn	: in std_logic;
		s00_axi_awaddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in std_logic;
		s00_axi_araddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in std_logic
	);
end tc_selector_v1_0;

architecture arch_imp of tc_selector_v1_0 is

	-- component declaration
	component tc_selector_v1_0_S00_AXI is
		generic (
		C_NB_PULSES         : integer   := 10;
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 7
		);
		port (
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic;

        -- Antenna AXI Stream Interfaces
        ant_axis_aresetn        : in std_logic;
        ant_axis_aclk           : in std_logic;
        
        ant0_axis_tdata         : in std_logic_vector(63 downto 0);
        ant0_axis_tvalid        : in std_logic;
        ant0_axis_tready        : out std_logic;
        
        ant1_axis_tdata         : in std_logic_vector(63 downto 0);
        ant1_axis_tvalid        : in std_logic;
        ant1_axis_tready        : out std_logic;
                
        ant2_axis_tdata         : in std_logic_vector(63 downto 0);
        ant2_axis_tvalid        : in std_logic;
        ant2_axis_tready        : out std_logic;
        
        ant3_axis_tdata         : in std_logic_vector(63 downto 0);
        ant3_axis_tvalid        : in std_logic;
        ant3_axis_tready        : out std_logic;
        
        ant_fifo_ovfl           : out std_logic;
        ant_fifo_unfl           : out std_logic;

        -- Internal Pulse FIFOs flags
        prev_pulse_fifo_ovfl    : out std_logic;
        prev_pulse_fifo_unfl    : out std_logic;
        curr_pulse_fifo_ovfl    : out std_logic;
        curr_pulse_fifo_unfl    : out std_logic;
        
        -- AIE Covariance Kernels Interfaces 
        cov_axis_aclk           : in std_logic;
        cov_axis_aresetn        : in std_logic;
        
        cov0_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov0_prev_axis_tvalid   : out std_logic;
        cov0_prev_axis_tready   : in std_logic;
        
        cov0_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov0_curr_axis_tvalid   : out std_logic;
        cov0_curr_axis_tready   : in std_logic;
        
        cov1_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov1_prev_axis_tvalid   : out std_logic;
        cov1_prev_axis_tready   : in std_logic;
        
        cov1_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov1_curr_axis_tvalid   : out std_logic;
        cov1_curr_axis_tready   : in std_logic;       
        
        cov2_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov2_prev_axis_tvalid   : out std_logic;
        cov2_prev_axis_tready   : in std_logic;
        
        cov2_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov2_curr_axis_tvalid   : out std_logic;
        cov2_curr_axis_tready   : in std_logic;
        
        cov3_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov3_prev_axis_tvalid   : out std_logic;
        cov3_prev_axis_tready   : in std_logic;
        
        cov3_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov3_curr_axis_tvalid   : out std_logic;
        cov3_curr_axis_tready   : in std_logic;
        
        cov4_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov4_prev_axis_tvalid   : out std_logic;
        cov4_prev_axis_tready   : in std_logic;
        
        cov4_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov4_curr_axis_tvalid   : out std_logic;
        cov4_curr_axis_tready   : in std_logic;
        
        cov5_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov5_prev_axis_tvalid   : out std_logic;
        cov5_prev_axis_tready   : in std_logic;
              
        cov5_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov5_curr_axis_tvalid   : out std_logic;
        cov5_curr_axis_tready   : in std_logic;
           
        cov6_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov6_prev_axis_tvalid   : out std_logic;
        cov6_prev_axis_tready   : in std_logic;
         
        cov6_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov6_curr_axis_tvalid   : out std_logic;
        cov6_curr_axis_tready   : in std_logic;

        cov7_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov7_prev_axis_tvalid   : out std_logic;
        cov7_prev_axis_tready   : in std_logic;
                       
        cov7_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov7_curr_axis_tvalid   : out std_logic;
        cov7_curr_axis_tready   : in std_logic;

        cov8_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov8_prev_axis_tvalid   : out std_logic;
        cov8_prev_axis_tready   : in std_logic;
        
        cov8_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov8_curr_axis_tvalid   : out std_logic;
        cov8_curr_axis_tready   : in std_logic;
        
        cov9_prev_axis_tdata    : out std_logic_vector(31 downto 0);
        cov9_prev_axis_tvalid   : out std_logic;
        cov9_prev_axis_tready   : in std_logic;
        
        cov9_curr_axis_tdata    : out std_logic_vector(31 downto 0);
        cov9_curr_axis_tvalid   : out std_logic;
        cov9_curr_axis_tready   : in std_logic;
        
        cov10_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov10_prev_axis_tvalid  : out std_logic;
        cov10_prev_axis_tready  : in std_logic;
        
        cov10_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov10_curr_axis_tvalid  : out std_logic;
        cov10_curr_axis_tready  : in std_logic;
          
        cov11_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov11_prev_axis_tvalid  : out std_logic;
        cov11_prev_axis_tready  : in std_logic;
        
        cov11_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov11_curr_axis_tvalid  : out std_logic;
        cov11_curr_axis_tready  : in std_logic;
        
        cov12_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov12_prev_axis_tvalid  : out std_logic;
        cov12_prev_axis_tready  : in std_logic;
        
        cov12_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov12_curr_axis_tvalid  : out std_logic;
        cov12_curr_axis_tready  : in std_logic;
        
        cov13_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov13_prev_axis_tvalid  : out std_logic;
        cov13_prev_axis_tready  : in std_logic;
        
        cov13_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov13_curr_axis_tvalid  : out std_logic;
        cov13_curr_axis_tready  : in std_logic;
                
        cov14_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov14_prev_axis_tvalid  : out std_logic;
        cov14_prev_axis_tready  : in std_logic;
        
        cov14_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov14_curr_axis_tvalid  : out std_logic;
        cov14_curr_axis_tready  : in std_logic;
        
        cov15_prev_axis_tdata   : out std_logic_vector(31 downto 0);
        cov15_prev_axis_tvalid  : out std_logic;
        cov15_prev_axis_tready  : in std_logic;
        
        cov15_curr_axis_tdata   : out std_logic_vector(31 downto 0);
        cov15_curr_axis_tvalid  : out std_logic;
        cov15_curr_axis_tready  : in std_logic
		);
	end component tc_selector_v1_0_S00_AXI;

begin

-- Instantiation of Axi Bus Interface S00_AXI
tc_selector_v1_0_S00_AXI_inst : tc_selector_v1_0_S00_AXI
	generic map (
        C_NB_PULSES         => C_NB_PULSES,
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
		S_AXI_ACLK	=> s00_axi_aclk,
		S_AXI_ARESETN	=> s00_axi_aresetn,
		S_AXI_AWADDR	=> s00_axi_awaddr,
		S_AXI_AWPROT	=> s00_axi_awprot,
		S_AXI_AWVALID	=> s00_axi_awvalid,
		S_AXI_AWREADY	=> s00_axi_awready,
		S_AXI_WDATA	=> s00_axi_wdata,
		S_AXI_WSTRB	=> s00_axi_wstrb,
		S_AXI_WVALID	=> s00_axi_wvalid,
		S_AXI_WREADY	=> s00_axi_wready,
		S_AXI_BRESP	=> s00_axi_bresp,
		S_AXI_BVALID	=> s00_axi_bvalid,
		S_AXI_BREADY	=> s00_axi_bready,
		S_AXI_ARADDR	=> s00_axi_araddr,
		S_AXI_ARPROT	=> s00_axi_arprot,
		S_AXI_ARVALID	=> s00_axi_arvalid,
		S_AXI_ARREADY	=> s00_axi_arready,
		S_AXI_RDATA	=> s00_axi_rdata,
		S_AXI_RRESP	=> s00_axi_rresp,
		S_AXI_RVALID	=> s00_axi_rvalid,
		S_AXI_RREADY	=> s00_axi_rready,

	-- Add user logic here
        -- Antenna AXI Stream Interfaces
        ant_axis_aresetn        => ant_axis_aresetn,
        ant_axis_aclk           => ant_axis_aclk,
        
        ant0_axis_tdata         => ant0_axis_tdata,
        ant0_axis_tvalid        => ant0_axis_tvalid,
        ant0_axis_tready        => ant0_axis_tready,
        
        ant1_axis_tdata         => ant1_axis_tdata,
        ant1_axis_tvalid        => ant1_axis_tvalid,
        ant1_axis_tready        => ant1_axis_tready,
                
        ant2_axis_tdata         => ant2_axis_tdata,
        ant2_axis_tvalid        => ant2_axis_tvalid,
        ant2_axis_tready        => ant2_axis_tready,

        ant3_axis_tdata         => ant3_axis_tdata,
        ant3_axis_tvalid        => ant3_axis_tvalid,
        ant3_axis_tready        => ant3_axis_tready,
        
        ant_fifo_ovfl           => ant_fifo_ovfl,
        ant_fifo_unfl           => ant_fifo_unfl,

        -- Internal Pulse FIFOs flags
        prev_pulse_fifo_ovfl    => prev_pulse_fifo_ovfl,
        prev_pulse_fifo_unfl    => prev_pulse_fifo_unfl,
        curr_pulse_fifo_ovfl    => curr_pulse_fifo_ovfl,
        curr_pulse_fifo_unfl    => curr_pulse_fifo_unfl,
               
        -- AIE Covariance Kernels Interfaces 
        cov_axis_aclk           => cov_axis_aclk,
        cov_axis_aresetn        => cov_axis_aresetn,

        cov0_prev_axis_tdata    => cov0_prev_axis_tdata,
        cov0_prev_axis_tvalid   => cov0_prev_axis_tvalid,
        cov0_prev_axis_tready   => cov0_prev_axis_tready,
        
        cov0_curr_axis_tdata    => cov0_curr_axis_tdata,
        cov0_curr_axis_tvalid   => cov0_curr_axis_tvalid,
        cov0_curr_axis_tready   => cov0_curr_axis_tready,
        
        cov1_prev_axis_tdata    => cov1_prev_axis_tdata,
        cov1_prev_axis_tvalid   => cov1_prev_axis_tvalid,
        cov1_prev_axis_tready   => cov1_prev_axis_tready,
        
        cov1_curr_axis_tdata    => cov1_curr_axis_tdata,
        cov1_curr_axis_tvalid   => cov1_curr_axis_tvalid,
        cov1_curr_axis_tready   => cov1_curr_axis_tready,     
        
        cov2_prev_axis_tdata    => cov2_prev_axis_tdata,
        cov2_prev_axis_tvalid   => cov2_prev_axis_tvalid,
        cov2_prev_axis_tready   => cov2_prev_axis_tready,
        
        cov2_curr_axis_tdata    => cov2_curr_axis_tdata,
        cov2_curr_axis_tvalid   => cov2_curr_axis_tvalid,
        cov2_curr_axis_tready   => cov2_curr_axis_tready,
        
        cov3_prev_axis_tdata    => cov3_prev_axis_tdata,
        cov3_prev_axis_tvalid   => cov3_prev_axis_tvalid,
        cov3_prev_axis_tready   => cov3_prev_axis_tready,
        
        cov3_curr_axis_tdata    => cov3_curr_axis_tdata,
        cov3_curr_axis_tvalid   => cov3_curr_axis_tvalid,
        cov3_curr_axis_tready   => cov3_curr_axis_tready,
        
        cov4_prev_axis_tdata    => cov4_prev_axis_tdata,
        cov4_prev_axis_tvalid   => cov4_prev_axis_tvalid,
        cov4_prev_axis_tready   => cov4_prev_axis_tready,
        
        cov4_curr_axis_tdata    => cov4_curr_axis_tdata,
        cov4_curr_axis_tvalid   => cov4_curr_axis_tvalid,
        cov4_curr_axis_tready   => cov4_curr_axis_tready,
        
        cov5_prev_axis_tdata    => cov5_prev_axis_tdata,
        cov5_prev_axis_tvalid   => cov5_prev_axis_tvalid,
        cov5_prev_axis_tready   => cov5_prev_axis_tready,
              
        cov5_curr_axis_tdata    => cov5_curr_axis_tdata,
        cov5_curr_axis_tvalid   => cov5_curr_axis_tvalid,
        cov5_curr_axis_tready   => cov5_curr_axis_tready,
           
        cov6_prev_axis_tdata    => cov6_prev_axis_tdata,
        cov6_prev_axis_tvalid   => cov6_prev_axis_tvalid,
        cov6_prev_axis_tready   => cov6_prev_axis_tready,
         
        cov6_curr_axis_tdata    => cov6_curr_axis_tdata,
        cov6_curr_axis_tvalid   => cov6_curr_axis_tvalid,
        cov6_curr_axis_tready   => cov6_curr_axis_tready,

        cov7_prev_axis_tdata    => cov7_prev_axis_tdata,
        cov7_prev_axis_tvalid   => cov7_prev_axis_tvalid,
        cov7_prev_axis_tready   => cov7_prev_axis_tready,
                       
        cov7_curr_axis_tdata    => cov7_curr_axis_tdata,
        cov7_curr_axis_tvalid   => cov7_curr_axis_tvalid,
        cov7_curr_axis_tready   => cov7_curr_axis_tready,

        cov8_prev_axis_tdata    => cov8_prev_axis_tdata,
        cov8_prev_axis_tvalid   => cov8_prev_axis_tvalid,
        cov8_prev_axis_tready   => cov8_prev_axis_tready,
        
        cov8_curr_axis_tdata    => cov8_curr_axis_tdata,
        cov8_curr_axis_tvalid   => cov8_curr_axis_tvalid,
        cov8_curr_axis_tready   => cov8_curr_axis_tready,
        
        cov9_prev_axis_tdata    => cov9_prev_axis_tdata,
        cov9_prev_axis_tvalid   => cov9_prev_axis_tvalid,
        cov9_prev_axis_tready   => cov9_prev_axis_tready,
        
        cov9_curr_axis_tdata    => cov9_curr_axis_tdata,
        cov9_curr_axis_tvalid   => cov9_curr_axis_tvalid,
        cov9_curr_axis_tready   => cov9_curr_axis_tready,
        
        cov10_prev_axis_tdata   => cov10_prev_axis_tdata,
        cov10_prev_axis_tvalid  => cov10_prev_axis_tvalid,
        cov10_prev_axis_tready  => cov10_prev_axis_tready,
        
        cov10_curr_axis_tdata   => cov10_curr_axis_tdata,
        cov10_curr_axis_tvalid  => cov10_curr_axis_tvalid,
        cov10_curr_axis_tready  => cov10_curr_axis_tready,
          
        cov11_prev_axis_tdata   => cov11_prev_axis_tdata,
        cov11_prev_axis_tvalid  => cov11_prev_axis_tvalid,
        cov11_prev_axis_tready  => cov11_prev_axis_tready,
        
        cov11_curr_axis_tdata   => cov11_curr_axis_tdata,
        cov11_curr_axis_tvalid  => cov11_curr_axis_tvalid,
        cov11_curr_axis_tready  => cov11_curr_axis_tready,
        
        cov12_prev_axis_tdata   => cov12_prev_axis_tdata,
        cov12_prev_axis_tvalid  => cov12_prev_axis_tvalid,
        cov12_prev_axis_tready  => cov12_prev_axis_tready,
        
        cov12_curr_axis_tdata   => cov12_curr_axis_tdata,
        cov12_curr_axis_tvalid  => cov12_curr_axis_tvalid,
        cov12_curr_axis_tready  => cov12_curr_axis_tready,
        
        cov13_prev_axis_tdata   => cov13_prev_axis_tdata,
        cov13_prev_axis_tvalid  => cov13_prev_axis_tvalid,
        cov13_prev_axis_tready  => cov13_prev_axis_tready,
        
        cov13_curr_axis_tdata   => cov13_curr_axis_tdata,
        cov13_curr_axis_tvalid  => cov13_curr_axis_tvalid,
        cov13_curr_axis_tready  => cov13_curr_axis_tready,
                
        cov14_prev_axis_tdata   => cov14_prev_axis_tdata,
        cov14_prev_axis_tvalid  => cov14_prev_axis_tvalid,
        cov14_prev_axis_tready  => cov14_prev_axis_tready,
        
        cov14_curr_axis_tdata   => cov14_curr_axis_tdata,
        cov14_curr_axis_tvalid  => cov14_curr_axis_tvalid,
        cov14_curr_axis_tready  => cov14_curr_axis_tready,
        
        cov15_prev_axis_tdata   => cov15_prev_axis_tdata,
        cov15_prev_axis_tvalid  => cov15_prev_axis_tvalid,
        cov15_prev_axis_tready  => cov15_prev_axis_tready,
        
        cov15_curr_axis_tdata   => cov15_curr_axis_tdata,
        cov15_curr_axis_tvalid  => cov15_curr_axis_tvalid,
        cov15_curr_axis_tready  => cov15_curr_axis_tready    

	-- User logic ends
	);

end arch_imp;
