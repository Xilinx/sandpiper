--
--Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
--SPDX-License-Identifier: MIT
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

Library xpm;
use xpm.vcomponents.all;

entity tc_selector_v1_0_S00_AXI is
	generic (
		-- Users to add parameters here
        C_NB_PULSES         : integer := 10;
		-- User parameters ends
		-- Do not modify the parameters beyond this line

		-- Width of S_AXI data bus
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		-- Width of S_AXI address bus
		C_S_AXI_ADDR_WIDTH	: integer	:= 7
	);
	port (
		-- Users to add ports here

		-- User ports ends
		-- Do not modify the ports beyond this line

		-- Global Clock Signal
		S_AXI_ACLK	: in std_logic;
		-- Global Reset Signal. This Signal is Active LOW
		S_AXI_ARESETN	: in std_logic;
		-- Write address (issued by master, acceped by Slave)
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Write channel Protection type. This signal indicates the
    		-- privilege and security level of the transaction, and whether
    		-- the transaction is a data access or an instruction access.
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		-- Write address valid. This signal indicates that the master signaling
    		-- valid write address and control information.
		S_AXI_AWVALID	: in std_logic;
		-- Write address ready. This signal indicates that the slave is ready
    		-- to accept an address and associated control signals.
		S_AXI_AWREADY	: out std_logic;
		-- Write data (issued by master, acceped by Slave) 
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Write strobes. This signal indicates which byte lanes hold
    		-- valid data. There is one write strobe bit for each eight
    		-- bits of the write data bus.    
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		-- Write valid. This signal indicates that valid write
    		-- data and strobes are available.
		S_AXI_WVALID	: in std_logic;
		-- Write ready. This signal indicates that the slave
    		-- can accept the write data.
		S_AXI_WREADY	: out std_logic;
		-- Write response. This signal indicates the status
    		-- of the write transaction.
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		-- Write response valid. This signal indicates that the channel
    		-- is signaling a valid write response.
		S_AXI_BVALID	: out std_logic;
		-- Response ready. This signal indicates that the master
    		-- can accept a write response.
		S_AXI_BREADY	: in std_logic;
		-- Read address (issued by master, acceped by Slave)
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		-- Protection type. This signal indicates the privilege
    		-- and security level of the transaction, and whether the
    		-- transaction is a data access or an instruction access.
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		-- Read address valid. This signal indicates that the channel
    		-- is signaling valid read address and control information.
		S_AXI_ARVALID	: in std_logic;
		-- Read address ready. This signal indicates that the slave is
    		-- ready to accept an address and associated control signals.
		S_AXI_ARREADY	: out std_logic;
		-- Read data (issued by slave)
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		-- Read response. This signal indicates the status of the
    		-- read transfer.
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		-- Read valid. This signal indicates that the channel is
    		-- signaling the required read data.
		S_AXI_RVALID	: out std_logic;
		-- Read ready. This signal indicates that the master can
    		-- accept the read data and response information.
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
end tc_selector_v1_0_S00_AXI;

architecture arch_imp of tc_selector_v1_0_S00_AXI is

	-- AXI4LITE signals
	signal axi_awaddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_awready	: std_logic;
	signal axi_wready	: std_logic;
	signal axi_bresp	: std_logic_vector(1 downto 0);
	signal axi_bvalid	: std_logic;
	signal axi_araddr	: std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
	signal axi_arready	: std_logic;
	signal axi_rdata	: std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal axi_rresp	: std_logic_vector(1 downto 0);
	signal axi_rvalid	: std_logic;

	-- Example-specific design signals
	-- local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	-- ADDR_LSB is used for addressing 32/64 bit registers/memories
	-- ADDR_LSB = 2 for 32 bits (n downto 2)
	-- ADDR_LSB = 3 for 64 bits (n downto 3)
	constant ADDR_LSB  : integer := (C_S_AXI_DATA_WIDTH/32)+ 1;
	constant OPT_MEM_ADDR_BITS : integer := 4;
	------------------------------------------------
	---- Signals for user logic register space example
	--------------------------------------------------
	---- Number of Slave Registers 32
	signal slv_reg0	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg1	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg2	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg3	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg4	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg5	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg6	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg7	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg8	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg9	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg10	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg11	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg12	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg13	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg14	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg15	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg16	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg17	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg18	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg19	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg20	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg21	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg22	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg23	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg24	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg25	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg26	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg27	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg28	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg29	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg30	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg31	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal slv_reg_rden	: std_logic;
	signal slv_reg_wren	: std_logic;
	signal reg_data_out	:std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
	signal byte_index	: integer;
	signal aw_en	: std_logic;

    -- Registers
    --
    signal L                    : unsigned(15 downto 0);    -- Nbr of samples (fast-time ticks)
    signal nb_cuboids           : unsigned(3 downto 0);     -- Total number of cuboids 

    type GUARD_CELL is array (integer range <>) of std_logic_vector(15 downto 0);
    signal nb_guard_cell        : GUARD_CELL(0 to 9);       --  Nbr of Guard Cells

    type TRAINING_CELL is array (integer range <>) of std_logic_vector(15 downto 0);
    signal nb_training_cell     : TRAINING_CELL(0 to 9);    --  Nbr of training cells

    type TARGET_RANGE is array (integer range <>) of std_logic_vector(15 downto 0);
    signal target_range_idx     : TARGET_RANGE(0 to 9);     --  tRangeIdx := the target range index  >= 1, <= L, for up to 10 Cuboids
    
    -- Antenma Sync FIFOs 
    --
    signal ant_fifo_srst        : std_logic;
        
    signal ant_fifo_empty       : std_logic;
    signal ant_fifo_rd_en       : std_logic;
    signal ant_data_valid       : std_logic;
    signal ant_fifo_start_rd    : std_logic;
    
    signal ant0_fifo_wr_rst_busy: std_logic;
    signal ant0_fifo_empty      : std_logic;
    signal ant0_fifo_full       : std_logic;
    signal ant0_fifo_dout       : std_logic_vector(31 downto 0);
    signal ant0_fifo_overflow   : std_logic;
    signal ant0_fifo_underflow  : std_logic;
    
    signal ant1_fifo_wr_rst_busy: std_logic;
    signal ant1_fifo_empty      : std_logic;
    signal ant1_fifo_full       : std_logic;
    signal ant1_fifo_dout       : std_logic_vector(31 downto 0);
    signal ant1_fifo_overflow   : std_logic;
    signal ant1_fifo_underflow  : std_logic;

    signal ant2_fifo_wr_rst_busy: std_logic;
    signal ant2_fifo_empty      : std_logic;
    signal ant2_fifo_full       : std_logic;
    signal ant2_fifo_dout       : std_logic_vector(31 downto 0);
    signal ant2_fifo_overflow   : std_logic;
    signal ant2_fifo_underflow  : std_logic;
    
    signal ant3_fifo_wr_rst_busy: std_logic;
    signal ant3_fifo_empty      : std_logic;
    signal ant3_fifo_full       : std_logic;
    signal ant3_fifo_dout       : std_logic_vector(31 downto 0);
    signal ant3_fifo_overflow   : std_logic;        
    signal ant3_fifo_underflow  : std_logic;
    
    -- Training Selector Logic
    --
    type tc_sel_fsm is (
        idle,
        process_cuboid
    );
    
    signal tc_sel_state         : tc_sel_fsm;
    
    signal samples_cnt          : unsigned(15 downto 0);
    signal cuboids_cnt          : integer range 0 to 9;
    signal pulse_cnt            : integer range 0 to C_NB_PULSES;
    
    signal tc_set0_active       : std_logic := '0';
    signal tc_set1_active       : std_logic := '0';
    signal tc_set2_active       : std_logic := '0';
    signal tc_active            : std_logic := '0';
    
    -- Training Cell Sets Initialization Logic
    --
    type tc_set_fsm is (
        idle,
        latch_settings,
        compute_tc
    );
    
    signal tc_set_state         : tc_set_fsm;
    
    signal set_tc_sets          : std_logic := '1';

    signal gC                   : unsigned(15 downto 0);
    signal tC                   : unsigned(15 downto 0);
    signal tRangeIdx            : unsigned(15 downto 0);
    
    signal start_idx_0          : unsigned(15 downto 0);
    signal stop_idx_0           : unsigned(15 downto 0);

    signal start_idx_1          : unsigned(15 downto 0);
    signal stop_idx_1           : unsigned(15 downto 0);
    
    signal start_idx_2          : unsigned(15 downto 0);
    signal stop_idx_2           : unsigned(15 downto 0);
    
    -- Pulse FIFOs (Previous and Current)
    --
    signal pulse_fifo_srst      : std_logic;
        
    signal pulse_fifo_data      : std_logic_vector(127 downto 0);
    signal pulse_fifo_wen       : std_logic_vector(15 downto 0) := X"0001";

    type PULSE_FIFO_DOUT is array (integer range <>) of std_logic_vector(31 downto 0);
    signal prev_pulse_fifo_dout : PULSE_FIFO_DOUT(0 to 15);
    signal prev_pulse_fifo_wr_en: std_logic_vector(15 downto 0);
    signal prev_pulse_fifo_rd_en: std_logic_vector(15 downto 0);
    signal prev_pulse_fifo_data_valid : std_logic_vector(15 downto 0);
    signal prev_pulse_fifo_empty : std_logic_vector(15 downto 0);
    signal prev_pulse_fifo_overflow : std_logic_vector(15 downto 0);
    signal prev_pulse_fifo_underflow : std_logic_vector(15 downto 0);
     
    signal curr_pulse_fifo_dout : PULSE_FIFO_DOUT(0 to 15);
    signal curr_pulse_fifo_wr_en: std_logic_vector(15 downto 0);
    signal curr_pulse_fifo_rd_en: std_logic_vector(15 downto 0);
    signal curr_pulse_fifo_data_valid : std_logic_vector(15 downto 0);
    signal curr_pulse_fifo_empty : std_logic_vector(15 downto 0);
    signal curr_pulse_fifo_overflow : std_logic_vector(15 downto 0);
    signal curr_pulse_fifo_underflow : std_logic_vector(15 downto 0);



    
begin
	-- I/O Connections assignments

	S_AXI_AWREADY	<= axi_awready;
	S_AXI_WREADY	<= axi_wready;
	S_AXI_BRESP	<= axi_bresp;
	S_AXI_BVALID	<= axi_bvalid;
	S_AXI_ARREADY	<= axi_arready;
	S_AXI_RDATA	<= axi_rdata;
	S_AXI_RRESP	<= axi_rresp;
	S_AXI_RVALID	<= axi_rvalid;
	-- Implement axi_awready generation
	-- axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	-- de-asserted when reset is low.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awready <= '0';
	      aw_en <= '1';
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- slave is ready to accept write address when
	        -- there is a valid write address and write data
	        -- on the write address and data bus. This design 
	        -- expects no outstanding transactions. 
	           axi_awready <= '1';
	           aw_en <= '0';
	        elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then
	           aw_en <= '1';
	           axi_awready <= '0';
	      else
	        axi_awready <= '0';
	      end if;
	    end if;
	  end if;
	end process;

	-- Implement axi_awaddr latching
	-- This process is used to latch the address when both 
	-- S_AXI_AWVALID and S_AXI_WVALID are valid. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_awaddr <= (others => '0');
	    else
	      if (axi_awready = '0' and S_AXI_AWVALID = '1' and S_AXI_WVALID = '1' and aw_en = '1') then
	        -- Write Address latching
	        axi_awaddr <= S_AXI_AWADDR;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_wready generation
	-- axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	-- S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	-- de-asserted when reset is low. 

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_wready <= '0';
	    else
	      if (axi_wready = '0' and S_AXI_WVALID = '1' and S_AXI_AWVALID = '1' and aw_en = '1') then
	          -- slave is ready to accept write data when 
	          -- there is a valid write address and write data
	          -- on the write address and data bus. This design 
	          -- expects no outstanding transactions.           
	          axi_wready <= '1';
	      else
	        axi_wready <= '0';
	      end if;
	    end if;
	  end if;
	end process; 

	-- Implement memory mapped register select and write logic generation
	-- The write data is accepted and written to memory mapped registers when
	-- axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	-- select byte enables of slave registers while writing.
	-- These registers are cleared when reset (active low) is applied.
	-- Slave register write enable is asserted when valid address and data are available
	-- and the slave is ready to accept the write address and write data.
	slv_reg_wren <= axi_wready and S_AXI_WVALID and axi_awready and S_AXI_AWVALID ;

	process (S_AXI_ACLK)
	variable loc_addr :std_logic_vector(OPT_MEM_ADDR_BITS downto 0); 
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      slv_reg0 <= (others => '0');
	      slv_reg1 <= (others => '0');
	      slv_reg2 <= (others => '0');
	      slv_reg3 <= (others => '0');
	      slv_reg4 <= (others => '0');
	      slv_reg5 <= (others => '0');
	      slv_reg6 <= (others => '0');
	      slv_reg7 <= (others => '0');
	      slv_reg8 <= (others => '0');
	      slv_reg9 <= (others => '0');
	      slv_reg10 <= (others => '0');
	      slv_reg11 <= (others => '0');
	      slv_reg12 <= (others => '0');
	      slv_reg13 <= (others => '0');
	      slv_reg14 <= (others => '0');
	      slv_reg15 <= (others => '0');
	      slv_reg16 <= (others => '0');
	      slv_reg17 <= (others => '0');
	      slv_reg18 <= (others => '0');
	      slv_reg19 <= (others => '0');
	      slv_reg20 <= (others => '0');
	      slv_reg21 <= (others => '0');
	      slv_reg22 <= (others => '0');
	      slv_reg23 <= (others => '0');
	      slv_reg24 <= (others => '0');
	      slv_reg25 <= (others => '0');
	      slv_reg26 <= (others => '0');
	      slv_reg27 <= (others => '0');
	      slv_reg28 <= (others => '0');
	      slv_reg29 <= (others => '0');
	      slv_reg30 <= (others => '0');
	      slv_reg31 <= (others => '0');
	    else
	      loc_addr := axi_awaddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	      if (slv_reg_wren = '1') then
	        case loc_addr is
	          when b"00000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 0
	                slv_reg0(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 1
	                slv_reg1(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00010" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 2
	                slv_reg2(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 3
	                slv_reg3(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00100" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 4
	                slv_reg4(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00101" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 5
	                slv_reg5(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00110" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 6
	                slv_reg6(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"00111" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 7
	                slv_reg7(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 8
	                slv_reg8(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 9
	                slv_reg9(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01010" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 10
	                slv_reg10(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 11
	                slv_reg11(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01100" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 12
	                slv_reg12(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01101" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 13
	                slv_reg13(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01110" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 14
	                slv_reg14(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"01111" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 15
	                slv_reg15(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 16
	                slv_reg16(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 17
	                slv_reg17(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10010" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 18
	                slv_reg18(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 19
	                slv_reg19(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10100" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 20
	                slv_reg20(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10101" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 21
	                slv_reg21(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10110" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 22
	                slv_reg22(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"10111" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 23
	                slv_reg23(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11000" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 24
	                slv_reg24(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11001" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 25
	                slv_reg25(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11010" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 26
	                slv_reg26(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11011" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 27
	                slv_reg27(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11100" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 28
	                slv_reg28(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11101" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 29
	                slv_reg29(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11110" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 30
	                slv_reg30(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when b"11111" =>
	            for byte_index in 0 to (C_S_AXI_DATA_WIDTH/8-1) loop
	              if ( S_AXI_WSTRB(byte_index) = '1' ) then
	                -- Respective byte enables are asserted as per write strobes                   
	                -- slave registor 31
	                slv_reg31(byte_index*8+7 downto byte_index*8) <= S_AXI_WDATA(byte_index*8+7 downto byte_index*8);
	              end if;
	            end loop;
	          when others =>
	            slv_reg0 <= slv_reg0;
	            slv_reg1 <= slv_reg1;
	            slv_reg2 <= slv_reg2;
	            slv_reg3 <= slv_reg3;
	            slv_reg4 <= slv_reg4;
	            slv_reg5 <= slv_reg5;
	            slv_reg6 <= slv_reg6;
	            slv_reg7 <= slv_reg7;
	            slv_reg8 <= slv_reg8;
	            slv_reg9 <= slv_reg9;
	            slv_reg10 <= slv_reg10;
	            slv_reg11 <= slv_reg11;
	            slv_reg12 <= slv_reg12;
	            slv_reg13 <= slv_reg13;
	            slv_reg14 <= slv_reg14;
	            slv_reg15 <= slv_reg15;
	            slv_reg16 <= slv_reg16;
	            slv_reg17 <= slv_reg17;
	            slv_reg18 <= slv_reg18;
	            slv_reg19 <= slv_reg19;
	            slv_reg20 <= slv_reg20;
	            slv_reg21 <= slv_reg21;
	            slv_reg22 <= slv_reg22;
	            slv_reg23 <= slv_reg23;
	            slv_reg24 <= slv_reg24;
	            slv_reg25 <= slv_reg25;
	            slv_reg26 <= slv_reg26;
	            slv_reg27 <= slv_reg27;
	            slv_reg28 <= slv_reg28;
	            slv_reg29 <= slv_reg29;
	            slv_reg30 <= slv_reg30;
	            slv_reg31 <= slv_reg31;
	        end case;
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement write response logic generation
	-- The write response and response valid signals are asserted by the slave 
	-- when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	-- This marks the acceptance of address and indicates the status of 
	-- write transaction.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_bvalid  <= '0';
	      axi_bresp   <= "00"; --need to work more on the responses
	    else
	      if (axi_awready = '1' and S_AXI_AWVALID = '1' and axi_wready = '1' and S_AXI_WVALID = '1' and axi_bvalid = '0'  ) then
	        axi_bvalid <= '1';
	        axi_bresp  <= "00"; 
	      elsif (S_AXI_BREADY = '1' and axi_bvalid = '1') then   --check if bready is asserted while bvalid is high)
	        axi_bvalid <= '0';                                 -- (there is a possibility that bready is always asserted high)
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arready generation
	-- axi_arready is asserted for one S_AXI_ACLK clock cycle when
	-- S_AXI_ARVALID is asserted. axi_awready is 
	-- de-asserted when reset (active low) is asserted. 
	-- The read address is also latched when S_AXI_ARVALID is 
	-- asserted. axi_araddr is reset to zero on reset assertion.

	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then 
	    if S_AXI_ARESETN = '0' then
	      axi_arready <= '0';
	      axi_araddr  <= (others => '1');
	    else
	      if (axi_arready = '0' and S_AXI_ARVALID = '1') then
	        -- indicates that the slave has acceped the valid read address
	        axi_arready <= '1';
	        -- Read Address latching 
	        axi_araddr  <= S_AXI_ARADDR;           
	      else
	        axi_arready <= '0';
	      end if;
	    end if;
	  end if;                   
	end process; 

	-- Implement axi_arvalid generation
	-- axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	-- S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	-- data are available on the axi_rdata bus at this instance. The 
	-- assertion of axi_rvalid marks the validity of read data on the 
	-- bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	-- is deasserted on reset (active low). axi_rresp and axi_rdata are 
	-- cleared to zero on reset (active low).  
	process (S_AXI_ACLK)
	begin
	  if rising_edge(S_AXI_ACLK) then
	    if S_AXI_ARESETN = '0' then
	      axi_rvalid <= '0';
	      axi_rresp  <= "00";
	    else
	      if (axi_arready = '1' and S_AXI_ARVALID = '1' and axi_rvalid = '0') then
	        -- Valid read data is available at the read data bus
	        axi_rvalid <= '1';
	        axi_rresp  <= "00"; -- 'OKAY' response
	      elsif (axi_rvalid = '1' and S_AXI_RREADY = '1') then
	        -- Read data is accepted by the master
	        axi_rvalid <= '0';
	      end if;            
	    end if;
	  end if;
	end process;

	-- Implement memory mapped register select and read logic generation
	-- Slave register read enable is asserted when valid address is available
	-- and the slave is ready to accept the read address.
	slv_reg_rden <= axi_arready and S_AXI_ARVALID and (not axi_rvalid) ;

	process (slv_reg0, slv_reg1, slv_reg2, slv_reg3, slv_reg4, slv_reg5, slv_reg6, slv_reg7, slv_reg8, slv_reg9, slv_reg10, slv_reg11, slv_reg12, slv_reg13, slv_reg14, slv_reg15, slv_reg16, slv_reg17, slv_reg18, slv_reg19, slv_reg20, slv_reg21, slv_reg22, slv_reg23, slv_reg24, slv_reg25, slv_reg26, slv_reg27, slv_reg28, slv_reg29, slv_reg30, slv_reg31, axi_araddr, S_AXI_ARESETN, slv_reg_rden)
	variable loc_addr :std_logic_vector(OPT_MEM_ADDR_BITS downto 0);
	begin
	    -- Address decoding for reading registers
	    loc_addr := axi_araddr(ADDR_LSB + OPT_MEM_ADDR_BITS downto ADDR_LSB);
	    case loc_addr is
	      when b"00000" =>
	        reg_data_out <= slv_reg0;
	      when b"00001" =>
	        reg_data_out <= slv_reg1;
	      when b"00010" =>
	        reg_data_out <= slv_reg2;
	      when b"00011" =>
	        reg_data_out <= slv_reg3;
	      when b"00100" =>
	        reg_data_out <= slv_reg4;
	      when b"00101" =>
	        reg_data_out <= slv_reg5;
	      when b"00110" =>
	        reg_data_out <= slv_reg6;
	      when b"00111" =>
	        reg_data_out <= slv_reg7;
	      when b"01000" =>
	        reg_data_out <= slv_reg8;
	      when b"01001" =>
	        reg_data_out <= slv_reg9;
	      when b"01010" =>
	        reg_data_out <= slv_reg10;
	      when b"01011" =>
	        reg_data_out <= slv_reg11;
	      when b"01100" =>
	        reg_data_out <= slv_reg12;
	      when b"01101" =>
	        reg_data_out <= slv_reg13;
	      when b"01110" =>
	        reg_data_out <= slv_reg14;
	      when b"01111" =>
	        reg_data_out <= slv_reg15;
	      when b"10000" =>
	        reg_data_out <= slv_reg16;
	      when b"10001" =>
	        reg_data_out <= slv_reg17;
	      when b"10010" =>
	        reg_data_out <= slv_reg18;
	      when b"10011" =>
	        reg_data_out <= slv_reg19;
	      when b"10100" =>
	        reg_data_out <= slv_reg20;
	      when b"10101" =>
	        reg_data_out <= slv_reg21;
	      when b"10110" =>
	        reg_data_out <= slv_reg22;
	      when b"10111" =>
	        reg_data_out <= slv_reg23;
	      when b"11000" =>
	        reg_data_out <= slv_reg24;
	      when b"11001" =>
	        reg_data_out <= slv_reg25;
	      when b"11010" =>
	        reg_data_out <= slv_reg26;
	      when b"11011" =>
	        reg_data_out <= slv_reg27;
	      when b"11100" =>
	        reg_data_out <= slv_reg28;
	      when b"11101" =>
	        reg_data_out <= slv_reg29;
	      when b"11110" =>
	        reg_data_out <= slv_reg30;
	      when b"11111" =>
	        reg_data_out <= slv_reg31;
	      when others =>
	        reg_data_out  <= (others => '0');
	    end case;
	end process; 

	-- Output register or memory read data
	process( S_AXI_ACLK ) is
	begin
	  if (rising_edge (S_AXI_ACLK)) then
	    if ( S_AXI_ARESETN = '0' ) then
	      axi_rdata  <= (others => '0');
	    else
	      if (slv_reg_rden = '1') then
	        -- When there is a valid read address (S_AXI_ARVALID) with 
	        -- acceptance of read address by the slave (axi_arready), 
	        -- output the read dada 
	        -- Read address mux
	          axi_rdata <= reg_data_out;     -- register read data
	      end if;   
	    end if;
	  end if;
	end process;


	-- Add user logic here 
	
    ---------------------------------------------------------------------------
    -- Memory Decoding
    ---------------------------------------------------------------------------

    L                           <= unsigned(slv_reg1(15 downto 0));
    nb_cuboids                  <= unsigned(slv_reg1(19 downto 16));
    nb_guard_cell(0)            <= slv_reg2(15 downto 0);
    nb_training_cell(0)         <= slv_reg3(15 downto 0);
    target_range_idx(0)         <= slv_reg4(15 downto 0);
    nb_guard_cell(1)            <= slv_reg5(15 downto 0);
    nb_training_cell(1)         <= slv_reg6(15 downto 0);    
    target_range_idx(1)         <= slv_reg7(15 downto 0);
    nb_guard_cell(2)            <= slv_reg8(15 downto 0);
    nb_training_cell(2)         <= slv_reg9(15 downto 0);
    target_range_idx(2)         <= slv_reg10(15 downto 0);
    nb_guard_cell(3)            <= slv_reg11(15 downto 0);
    nb_training_cell(3)         <= slv_reg12(15 downto 0);
    target_range_idx(3)         <= slv_reg13(15 downto 0);
    nb_guard_cell(4)            <= slv_reg14(15 downto 0);
    nb_training_cell(4)         <= slv_reg15(15 downto 0);
    target_range_idx(4)         <= slv_reg16(15 downto 0);
    nb_guard_cell(5)            <= slv_reg17(15 downto 0);
    nb_training_cell(5)         <= slv_reg18(15 downto 0);
    target_range_idx(5)         <= slv_reg19(15 downto 0);
    nb_guard_cell(6)            <= slv_reg20(15 downto 0);
    nb_training_cell(6)         <= slv_reg21(15 downto 0);
    target_range_idx(6)         <= slv_reg22(15 downto 0);
    nb_guard_cell(7)            <= slv_reg23(15 downto 0);
    nb_training_cell(7)         <= slv_reg24(15 downto 0);
    target_range_idx(7)         <= slv_reg25(15 downto 0);
    nb_guard_cell(8)            <= slv_reg26(15 downto 0);
    nb_training_cell(8)         <= slv_reg27(15 downto 0);
    target_range_idx(8)         <= slv_reg28(15 downto 0);
    nb_guard_cell(9)            <= slv_reg29(15 downto 0);
    nb_training_cell(9)         <= slv_reg30(15 downto 0);
    target_range_idx(9)         <= slv_reg31(15 downto 0);


    ---------------------------------------------------------------------------
    -- FIFOs to accomodate frequency and width changes.
    -- These FIFOs also align the Antenna JESD Interfaces
    ---------------------------------------------------------------------------

    ant_fifo_srst   <= not ant_axis_aresetn;

    -- 0 when all antenna FIFOs are not empty
    ant_fifo_empty  <= ant0_fifo_empty or ant1_fifo_empty or ant2_fifo_empty or ant3_fifo_empty;
    
    ant_fifo_rd_en  <= not(ant_fifo_empty) and (ant_fifo_start_rd);

    antenna0_sync_fifo : entity work.antenna_sync_fifo
      port map (
        srst                    => ant_fifo_srst,
        wr_rst_busy             => ant0_fifo_wr_rst_busy,
        wr_clk                  => ant_axis_aclk,
        rd_clk                  => cov_axis_aclk,
        din                     => ant0_axis_tdata,
        wr_en                   => ant0_axis_tvalid,
        rd_en                   => ant_fifo_rd_en,
        dout                    => ant0_fifo_dout,
        data_valid              => ant_data_valid,
        full                    => ant0_fifo_full,
        empty                   => ant0_fifo_empty,
        overflow                => ant0_fifo_overflow,
        underflow               => ant0_fifo_underflow
      );

    antenna1_sync_fifo : entity work.antenna_sync_fifo
      port map (
        srst                    => ant_fifo_srst,
        wr_rst_busy             => ant1_fifo_wr_rst_busy,
        wr_clk                  => ant_axis_aclk,
        rd_clk                  => cov_axis_aclk,
        din                     => ant1_axis_tdata,
        wr_en                   => ant1_axis_tvalid,
        rd_en                   => ant_fifo_rd_en,
        dout                    => ant1_fifo_dout,
        data_valid              => open,
        full                    => ant1_fifo_full,
        empty                   => ant1_fifo_empty,
        overflow                => ant1_fifo_overflow,
        underflow               => ant1_fifo_underflow
      );
      
    antenna2_sync_fifo : entity work.antenna_sync_fifo
      port map (
        srst                    => ant_fifo_srst,
        wr_rst_busy             => ant2_fifo_wr_rst_busy,
        wr_clk                  => ant_axis_aclk,
        rd_clk                  => cov_axis_aclk,
        din                     => ant2_axis_tdata,
        wr_en                   => ant2_axis_tvalid,
        rd_en                   => ant_fifo_rd_en,
        dout                    => ant2_fifo_dout,
        data_valid              => open,
        full                    => ant2_fifo_full,
        empty                   => ant2_fifo_empty,
        overflow                => ant2_fifo_overflow,
        underflow               => ant2_fifo_underflow
      );
      
    antenna3_sync_fifo : entity work.antenna_sync_fifo
      port map (
        srst                    => ant_fifo_srst,
        wr_rst_busy             => ant3_fifo_wr_rst_busy,
        wr_clk                  => ant_axis_aclk,
        rd_clk                  => cov_axis_aclk,
        din                     => ant3_axis_tdata,
        wr_en                   => ant3_axis_tvalid,
        rd_en                   => ant_fifo_rd_en,
        dout                    => ant3_fifo_dout,
        data_valid              => open,
        full                    => ant3_fifo_full,
        empty                   => ant3_fifo_empty,
        overflow                => ant3_fifo_overflow,
        underflow               => ant3_fifo_underflow
      );
    
    ant0_axis_tready    <= '1'; -- not ant0_fifo_wr_rst_busy;
    ant1_axis_tready    <= '1'; -- not ant1_fifo_wr_rst_busy;
    ant2_axis_tready    <= '1'; -- not ant2_fifo_wr_rst_busy;
    ant3_axis_tready    <= '1'; -- not ant3_fifo_wr_rst_busy;
    
    ant_fifo_ovfl       <= ant0_fifo_overflow or ant1_fifo_overflow or ant2_fifo_overflow or ant3_fifo_overflow;
    ant_fifo_unfl       <= ant0_fifo_underflow or ant1_fifo_underflow or ant2_fifo_underflow or ant3_fifo_underflow;


    ---------------------------------------------------------------------------
    -- Training Cell Selection
    ---------------------------------------------------------------------------
    
    process(cov_axis_aclk) is
    begin
        if rising_edge(cov_axis_aclk) then
            if (cov_axis_aresetn = '0') then
                samples_cnt                 <= (others => '0');
                pulse_cnt                   <= 0;
                cuboids_cnt                 <= 0;
                ant_fifo_start_rd           <= '0';
                set_tc_sets                 <= '1';
                tc_sel_state                <= idle;
            else
            
              case(tc_sel_state) is
                
                when idle =>
                    
                    if (ant_fifo_empty = '0') then
                        -- Start the process when all received antenna FIFOs are not empty
                        ant_fifo_start_rd   <= '1';
                        set_tc_sets         <= '0';
                        tc_sel_state        <= process_cuboid;
                    end if;


                when process_cuboid =>

                    -- For now stay here forever (until reset)
                    if (ant_data_valid = '1') then
                    
                        if (samples_cnt = L - 1) then
                            -- Last sample of a pulse
                            samples_cnt     <= (others => '0');
                            if (pulse_cnt = C_NB_PULSES - 1) then
                                -- Last sample of the last pulse
                                pulse_cnt   <= 0;
                            else
                                pulse_cnt   <= pulse_cnt + 1;
                            end if;
                        else
                            samples_cnt     <= samples_cnt + 1;
                        end if;
                     
                        if (samples_cnt = L - 4 and pulse_cnt = C_NB_PULSES - 1) then
                            -- Update the Training Cell Sets
                            set_tc_sets     <= '1';
                            -- Update the cuboid cuboid to the next one
                            if (cuboids_cnt = nb_cuboids - 1) then
                                cuboids_cnt <= 0;
                            else
                                cuboids_cnt <= cuboids_cnt + 1;
                            end if;
                                
                        else
                            set_tc_sets     <= '0';
                        end if;
                           
                    end if;
                
                
                when others =>
                
                    tc_sel_state            <= idle;

                end case;
                
            end if;
        end if;
    end process;


    ---------------------------------------------------------------------------
    -- Training Cell Sets Initialization
    ---------------------------------------------------------------------------
    
    process(cov_axis_aclk) is
    begin
        if rising_edge(cov_axis_aclk) then
            if (cov_axis_aresetn = '0') then
                gC                          <= (others => '0');
                tC                          <= (others => '0');
                tRangeIdx                   <= (others => '0');
                start_idx_0                 <= (others => '0');
                stop_idx_0                  <= (others => '0');
                start_idx_1                 <= (others => '0');
                stop_idx_1                  <= (others => '0');
                start_idx_2                 <= (others => '0');
                stop_idx_2                  <= (others => '0');
                tc_set_state                <= idle;
            else
            
              case(tc_set_state) is
                
                when idle =>
                    
                    if (set_tc_sets = '1') then
                        -- Start the process when all received antenna FIFOs are not empty
                        tc_set_state        <= latch_settings;
                    end if;


                when latch_settings =>
                    
                    gC                      <= unsigned(nb_guard_cell(cuboids_cnt));
                    tC                      <= unsigned(nb_training_cell(cuboids_cnt));
                    tRangeIdx               <= unsigned(target_range_idx(cuboids_cnt));
                                      
                    if (ant_fifo_start_rd = '0' or ant_data_valid = '1') then
                        tc_set_state        <= compute_tc;
                    end if;


                when compute_tc =>
  
                    --   The position of the taget range determines the number of contiguous
                    --   training sets
    
                    -- WRap to the start of cuboid
                    if (tRangeIdx + (gC+tC)/2 > L) then
                    
                        if (tRangeIdx + gC/2 >= L) then
                            start_idx_0     <= tRangeIdx + gC/2 - L;
                            stop_idx_0      <= tRangeIdx + (gC+tC)/2 - L - 1;
                            start_idx_1     <= tRangeIdx - (gC+tC)/2 - 1;
                            stop_idx_1      <= tRangeIdx - gC/2 - 2;
                            start_idx_2     <= (0 => '1', others => '0');
                            stop_idx_2      <= (others => '0');
                        else
                            start_idx_0     <= (others => '0');
                            stop_idx_0      <= tRangeIdx + (gC+tC)/2 - L - 1;
                            start_idx_1     <= tRangeIdx - (gC+tC)/2 - 1;
                            stop_idx_1      <= tRangeIdx - gC/2 - 2;
                            start_idx_2     <= tRangeIdx + gC/2;
                            stop_idx_2      <= L - 1;
                        end if;
                    
                    -- Wrap to the end of cuboid
                    elsif ((tRangeIdx+L) - (gC+tC)/2 < L) then
                    
                        if (gC/2 >= tRangeIdx - 1) then
                            start_idx_0     <= tRangeIdx + (gC/2);
                            stop_idx_0      <= tRangeIdx + (gC+tC)/2 - 1;
                            start_idx_1     <= tRangeIdx + L - (gC+tC)/2 - 1;
                            stop_idx_1      <= tRangeIdx + L - gC/2 - 2;
                            start_idx_2     <= (0 => '1', others => '0');
                            stop_idx_2      <= (others => '0');
                        else
                            start_idx_0     <= (others => '0');
                            stop_idx_0      <= tRangeIdx - gC/2 - 2;
                            start_idx_1     <= tRangeIdx + gC/2;
                            stop_idx_1      <= tRangeIdx + (gC+tC)/2 - 1;
                            start_idx_2     <= tRangeIdx + L - (gC+tC)/2 - 1;
                            stop_idx_2      <= L - 1;
                        end if;
                    
                    -- Training within the boundaries of the cuboid
                    else
                        start_idx_0         <= tRangeIdx - (gC+tC)/2 - 1;
                        stop_idx_0          <= tRangeIdx - gC/2 - 2;
                        start_idx_1         <= tRangeIdx + (gC/2);
                        stop_idx_1          <= tRangeIdx + (gC+tC)/2 - 1;
                        start_idx_2         <= (0 => '1', others => '0');
                        stop_idx_2          <= (others => '0');
                    end if;
      
                    if (ant_fifo_start_rd = '0' or ant_data_valid = '1') then
                        tc_set_state        <= idle;
                    end if;


                when others =>
                
                    tc_set_state            <= idle;

                end case;
                
            end if;
        end if;
    end process;
    
    tc_set0_active      <= '1' when samples_cnt >= start_idx_0 and samples_cnt <= stop_idx_0 else '0';
    tc_set1_active      <= '1' when samples_cnt >= start_idx_1 and samples_cnt <= stop_idx_1 else '0';
    tc_set2_active      <= '1' when samples_cnt >= start_idx_2 and samples_cnt <= stop_idx_2 else '0';

    tc_active           <= tc_set0_active or tc_set1_active or tc_set2_active;


    ---------------------------------------------------------------------------
    -- Pulse FIFOs: previous pulse
    -- These FIFOs stores a complete pulse and skip the last pulse of every cuboid
    -- With the current settings, L = 4000 (worst TC value), the FIFO depht should be 4000/16
    ---------------------------------------------------------------------------
    
    pulse_fifo_srst             <= not cov_axis_aresetn;
    
    pulse_fifo_data             <= ant3_fifo_dout & ant2_fifo_dout & ant1_fifo_dout & ant0_fifo_dout;

    -- We read the previous and current pulse FIFO at the same time
    --
    prev_pulse_fifo_rd_en(0)    <= not curr_pulse_fifo_empty(0)  and cov0_prev_axis_tready  and cov0_curr_axis_tready;
    prev_pulse_fifo_rd_en(1)    <= not curr_pulse_fifo_empty(1)  and cov1_prev_axis_tready  and cov1_curr_axis_tready;
    prev_pulse_fifo_rd_en(2)    <= not curr_pulse_fifo_empty(2)  and cov2_prev_axis_tready  and cov2_curr_axis_tready;
    prev_pulse_fifo_rd_en(3)    <= not curr_pulse_fifo_empty(3)  and cov3_prev_axis_tready  and cov3_curr_axis_tready;
    prev_pulse_fifo_rd_en(4)    <= not curr_pulse_fifo_empty(4)  and cov4_prev_axis_tready  and cov4_curr_axis_tready;
    prev_pulse_fifo_rd_en(5)    <= not curr_pulse_fifo_empty(5)  and cov5_prev_axis_tready  and cov5_curr_axis_tready;
    prev_pulse_fifo_rd_en(6)    <= not curr_pulse_fifo_empty(6)  and cov6_prev_axis_tready  and cov6_curr_axis_tready;
    prev_pulse_fifo_rd_en(7)    <= not curr_pulse_fifo_empty(7)  and cov7_prev_axis_tready  and cov7_curr_axis_tready;
    prev_pulse_fifo_rd_en(8)    <= not curr_pulse_fifo_empty(8)  and cov8_prev_axis_tready  and cov8_curr_axis_tready;
    prev_pulse_fifo_rd_en(9)    <= not curr_pulse_fifo_empty(9)  and cov9_prev_axis_tready  and cov9_curr_axis_tready;
    prev_pulse_fifo_rd_en(10)   <= not curr_pulse_fifo_empty(10) and cov10_prev_axis_tready and cov10_curr_axis_tready;
    prev_pulse_fifo_rd_en(11)   <= not curr_pulse_fifo_empty(11) and cov11_prev_axis_tready and cov11_curr_axis_tready;
    prev_pulse_fifo_rd_en(12)   <= not curr_pulse_fifo_empty(12) and cov12_prev_axis_tready and cov12_curr_axis_tready;
    prev_pulse_fifo_rd_en(13)   <= not curr_pulse_fifo_empty(13) and cov13_prev_axis_tready and cov13_curr_axis_tready;
    prev_pulse_fifo_rd_en(14)   <= not curr_pulse_fifo_empty(14) and cov14_prev_axis_tready and cov14_curr_axis_tready;
    prev_pulse_fifo_rd_en(15)   <= not curr_pulse_fifo_empty(15) and cov15_prev_axis_tready and cov15_curr_axis_tready;
    
    -- Pulse FIFOs are written sequentially. tC / 16 must be an integer.
    --
    
    process(cov_axis_aclk) is
    begin
        if rising_edge(cov_axis_aclk) then
            if (cov_axis_aresetn = '0') then
                pulse_fifo_wen     <= X"0001";
            elsif (ant_data_valid = '1' and tc_active = '1') then
                pulse_fifo_wen     <= pulse_fifo_wen(14 downto 0) & pulse_fifo_wen(15);
            end if;
        end if;
    end process;


    PREV_PULSE_FIFO : for cov_id in 0 to 15 generate
    
        prev_pulse_fifo_wr_en(cov_id) <= '1' when (ant_data_valid = '1') and (pulse_cnt /= C_NB_PULSES-1) and (pulse_fifo_wen(cov_id) = '1') and (tc_active = '1') else '0';
        
        prev_pulse_fifo_u : entity work.pulse_fifo
        generic map (
            FIFO_WRITE_DEPTH        => 256
        )
        port map (
            srst                    => pulse_fifo_srst,
            clk                     => cov_axis_aclk,
            din                     => pulse_fifo_data,
            wr_en                   => prev_pulse_fifo_wr_en(cov_id),
            rd_en                   => prev_pulse_fifo_rd_en(cov_id),
            dout                    => prev_pulse_fifo_dout(cov_id),
            data_valid              => prev_pulse_fifo_data_valid(cov_id),
            full                    => open,
            empty                   => prev_pulse_fifo_empty(cov_id),
            overflow                => prev_pulse_fifo_overflow(cov_id),
            underflow               => prev_pulse_fifo_underflow(cov_id)
        );
    
    end generate PREV_PULSE_FIFO;
    
    prev_pulse_fifo_ovfl    <= '0' when prev_pulse_fifo_overflow = X"0000" else '1';
    prev_pulse_fifo_unfl    <= '0' when prev_pulse_fifo_underflow = X"0000" else '1';
    
    
    ---------------------------------------------------------------------------
    -- Pulse FIFOs: current pulse
    -- These FIFOs stores a complete pulse and skip the last pulse of every cuboid
    ---------------------------------------------------------------------------
    
    -- We read the previous and current pulse FIFO at the same time
    --
    curr_pulse_fifo_rd_en   <= prev_pulse_fifo_rd_en;
    
    CURR_PULSE_FIFO : for cov_id in 0 to 15 generate
    
        curr_pulse_fifo_wr_en(cov_id) <= '1' when (ant_data_valid = '1') and (pulse_cnt /= 0) and (pulse_fifo_wen(cov_id) = '1') and (tc_active = '1') else '0';
        
        curr_pulse_fifo_u : entity work.pulse_fifo
        generic map (
            FIFO_WRITE_DEPTH        => 16
        )
        port map (
            srst                    => pulse_fifo_srst,
            clk                     => cov_axis_aclk,
            din                     => pulse_fifo_data,
            wr_en                   => curr_pulse_fifo_wr_en(cov_id),
            rd_en                   => curr_pulse_fifo_rd_en(cov_id),
            dout                    => curr_pulse_fifo_dout(cov_id),
            data_valid              => curr_pulse_fifo_data_valid(cov_id),
            full                    => open,
            empty                   => curr_pulse_fifo_empty(cov_id),
            overflow                => curr_pulse_fifo_overflow(cov_id),
            underflow               => curr_pulse_fifo_underflow(cov_id)
        );
    
    end generate CURR_PULSE_FIFO;
    
    curr_pulse_fifo_ovfl    <= '0' when curr_pulse_fifo_overflow = X"0000" else '1';
    curr_pulse_fifo_unfl    <= '0' when curr_pulse_fifo_underflow = X"0000" else '1';
        
        
    ---------------------------------------------------------------------------
    -- AIE Covariance Kernels Interfaces 
    ---------------------------------------------------------------------------

    cov0_prev_axis_tdata    <= prev_pulse_fifo_dout(0);
    cov0_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(0);
    
    cov0_curr_axis_tdata    <= curr_pulse_fifo_dout(0);
    cov0_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(0);
    
    cov1_prev_axis_tdata    <= prev_pulse_fifo_dout(1);
    cov1_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(1);
    
    cov1_curr_axis_tdata    <= curr_pulse_fifo_dout(1);
    cov1_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(1);
    
    cov2_prev_axis_tdata    <= prev_pulse_fifo_dout(2);
    cov2_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(2);
    
    cov2_curr_axis_tdata    <= curr_pulse_fifo_dout(2);
    cov2_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(2);
    
    cov3_prev_axis_tdata    <= prev_pulse_fifo_dout(3);
    cov3_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(3);
    
    cov3_curr_axis_tdata    <= curr_pulse_fifo_dout(3);
    cov3_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(3);
    
    cov4_prev_axis_tdata    <= prev_pulse_fifo_dout(4);
    cov4_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(4);
    
    cov4_curr_axis_tdata    <= curr_pulse_fifo_dout(4);
    cov4_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(4);
    
    cov5_prev_axis_tdata    <= prev_pulse_fifo_dout(5);
    cov5_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(5);
          
    cov5_curr_axis_tdata    <= curr_pulse_fifo_dout(5);
    cov5_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(5);
       
    cov6_prev_axis_tdata    <= prev_pulse_fifo_dout(6);
    cov6_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(6);
     
    cov6_curr_axis_tdata    <= curr_pulse_fifo_dout(6);
    cov6_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(6);

    cov7_prev_axis_tdata    <= prev_pulse_fifo_dout(7);
    cov7_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(7);
                   
    cov7_curr_axis_tdata    <= curr_pulse_fifo_dout(7);
    cov7_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(7);

    cov8_prev_axis_tdata    <= prev_pulse_fifo_dout(8);
    cov8_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(8);
    
    cov8_curr_axis_tdata    <= curr_pulse_fifo_dout(8);
    cov8_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(8);
    
    cov9_prev_axis_tdata    <= prev_pulse_fifo_dout(9);
    cov9_prev_axis_tvalid   <= prev_pulse_fifo_data_valid(9);
    
    cov9_curr_axis_tdata    <= curr_pulse_fifo_dout(9);
    cov9_curr_axis_tvalid   <= curr_pulse_fifo_data_valid(9);
    
    cov10_prev_axis_tdata   <= prev_pulse_fifo_dout(10);
    cov10_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(10);
    
    cov10_curr_axis_tdata   <= curr_pulse_fifo_dout(10);
    cov10_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(10);
      
    cov11_prev_axis_tdata   <= prev_pulse_fifo_dout(11);
    cov11_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(11);
    
    cov11_curr_axis_tdata   <= curr_pulse_fifo_dout(11);
    cov11_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(11);
    
    cov12_prev_axis_tdata   <= prev_pulse_fifo_dout(12);
    cov12_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(12);
    
    cov12_curr_axis_tdata   <= curr_pulse_fifo_dout(12);
    cov12_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(12);
    
    cov13_prev_axis_tdata   <= prev_pulse_fifo_dout(13);
    cov13_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(13);
    
    cov13_curr_axis_tdata   <= curr_pulse_fifo_dout(13);
    cov13_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(13);
            
    cov14_prev_axis_tdata   <= prev_pulse_fifo_dout(14);
    cov14_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(14);
    
    cov14_curr_axis_tdata   <= curr_pulse_fifo_dout(14);
    cov14_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(14);
    
    cov15_prev_axis_tdata   <= prev_pulse_fifo_dout(15);
    cov15_prev_axis_tvalid  <= prev_pulse_fifo_data_valid(15);
    
    cov15_curr_axis_tdata   <= curr_pulse_fifo_dout(15);
    cov15_curr_axis_tvalid  <= curr_pulse_fifo_data_valid(15);

    -- User logic ends

end arch_imp;
