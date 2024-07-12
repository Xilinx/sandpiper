--
--Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
--SPDX-License-Identifier: MIT
--

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

Library xpm;
use xpm.vcomponents.all;

library UNISIM;
use UNISIM.VComponents.all;


entity antenna_sync_fifo is
    port (
        srst                    : in std_logic;
        wr_rst_busy             : out std_logic;
        wr_clk                  : in std_logic;
        rd_clk                  : in std_logic;
        din                     : in std_logic_vector(63 downto 0);
        wr_en                   : in std_logic;
        rd_en                   : in std_logic;
        dout                    : out std_logic_vector(31 downto 0);
        data_valid              : out std_logic;
        full                    : out std_logic;
        empty                   : out std_logic;
        overflow                : out std_logic;
        underflow               : out std_logic
    );
end antenna_sync_fifo;

architecture Behavioral of antenna_sync_fifo is

begin


   -- xpm_fifo_async: Asynchronous FIFO
   -- Xilinx Parameterized Macro, version 2022.1

   xpm_fifo_async_inst : xpm_fifo_async
   generic map (
      CASCADE_HEIGHT            => 0,
      CDC_SYNC_STAGES           => 2,
      DOUT_RESET_VALUE          => "0",
      ECC_MODE                  => "no_ecc",
      FIFO_MEMORY_TYPE          => "auto",
      FIFO_READ_LATENCY         => 1,
      FIFO_WRITE_DEPTH          => 16,
      FULL_RESET_VALUE          => 0,
      PROG_EMPTY_THRESH         => 10,
      PROG_FULL_THRESH          => 10,
      RD_DATA_COUNT_WIDTH       => 1,
      READ_DATA_WIDTH           => 32,
      READ_MODE                 => "std",
      RELATED_CLOCKS            => 0,
      SIM_ASSERT_CHK            => 0,
      USE_ADV_FEATURES          => "1101",
    -- | USE_ADV_FEATURES     | String             | Default value = 0707.                                                   |
    -- |---------------------------------------------------------------------------------------------------------------------|
    -- | Enables data_valid, almost_empty, rd_data_count, prog_empty, underflow, wr_ack, almost_full, wr_data_count,         |
    -- | prog_full, overflow features.                                                                                       |
    -- |                                                                                                                     |
    -- |   Setting USE_ADV_FEATURES[0] to 1 enables overflow flag; Default value of this bit is 1                            |
    -- |   Setting USE_ADV_FEATURES[1] to 1 enables prog_full flag; Default value of this bit is 1                           |
    -- |   Setting USE_ADV_FEATURES[2] to 1 enables wr_data_count; Default value of this bit is 1                            |
    -- |   Setting USE_ADV_FEATURES[3] to 1 enables almost_full flag; Default value of this bit is 0                         |
    -- |   Setting USE_ADV_FEATURES[4] to 1 enables wr_ack flag; Default value of this bit is 0                              |
    -- |   Setting USE_ADV_FEATURES[8] to 1 enables underflow flag; Default value of this bit is 1                           |
    -- |   Setting USE_ADV_FEATURES[9] to 1 enables prog_empty flag; Default value of this bit is 1                          |
    -- |   Setting USE_ADV_FEATURES[10] to 1 enables rd_data_count; Default value of this bit is 1                           |
    -- |   Setting USE_ADV_FEATURES[11] to 1 enables almost_empty flag; Default value of this bit is 0                       |
    -- |   Setting USE_ADV_FEATURES[12] to 1 enables data_valid flag; Default value of this bit is 0                         |
    -- |---------------------------------------------------------------------------------------------------------------------|
      WAKEUP_TIME               => 0,
      WRITE_DATA_WIDTH          => 64,
      WR_DATA_COUNT_WIDTH       => 1
   )
   port map (
      almost_empty              => open,
      almost_full               => open,
      data_valid                => data_valid,
      dbiterr                   => open,
      dout                      => dout,
      empty                     => empty,
      full                      => full,
      overflow                  => overflow, 
      prog_empty                => open,
      prog_full                 => open,
      rd_data_count             => open,
      rd_rst_busy               => open,
      sbiterr                   => open,
      underflow                 => underflow,
      wr_ack                    => open,
      wr_data_count             => open,
      wr_rst_busy               => wr_rst_busy,
      din                       => din,
      injectdbiterr             => '0',
      injectsbiterr             => '0',
      rd_clk                    => rd_clk,
      rd_en                     => rd_en,
      rst                       => srst,
      sleep                     => '0',
      wr_clk                    => wr_clk,
      wr_en                     => wr_en
   );
   
end Behavioral;
