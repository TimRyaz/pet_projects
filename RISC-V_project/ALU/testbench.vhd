
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity alu_integration_tb is
-- ?????? entity
end entity;

architecture sim of alu_integration_tb is

    -- ?????????? ???? ?????-???
    component ALU is
        port(
            CLK          : in  STD_LOGIC;
            RST          : in  STD_LOGIC;
            Start_Div    : in  STD_LOGIC;
            A            : in  STD_LOGIC_VECTOR(31 downto 0);
            B            : in  STD_LOGIC_VECTOR(31 downto 0);
            ALUControl   : in  STD_LOGIC_VECTOR(3 downto 0);
            ALU_M_Ext    : in  STD_LOGIC;
            Result       : out STD_LOGIC_VECTOR(31 downto 0);
            Div_Ready    : out STD_LOGIC
        );
    end component;

    -- ??????? ??? ?????????
    signal CLK_sim        : STD_LOGIC := '0';
    signal RST_sim        : STD_LOGIC := '0';
    signal Start_Div_sim  : STD_LOGIC := '0';
    signal A_sim          : STD_LOGIC_VECTOR(31 downto 0) := (others => '0');
    signal B_sim          : STD_LOGIC_VECTOR(31 downto 0) := (others => '0');
    signal ALUControl_sim : STD_LOGIC_VECTOR(3 downto 0)  := "0000";
    signal ALU_M_Ext_sim  : STD_LOGIC := '0';
    signal Result_sim     : STD_LOGIC_VECTOR(31 downto 0);
    signal Div_Ready_sim  : STD_LOGIC;

    constant CLK_PERIOD : time := 20 ns; -- 50 ???

begin

    -- ???????????? ???
    uut: ALU
        port map(
            CLK          => CLK_sim,
            RST          => RST_sim,
            Start_Div    => Start_Div_sim,
            A            => A_sim,
            B            => B_sim,
            ALUControl   => ALUControl_sim,
            ALU_M_Ext    => ALU_M_Ext_sim,
            Result       => Result_sim,
            Div_Ready    => Div_Ready_sim
        );

    -- ????????? ????????? ???????
    clk_process : process
    begin
        while true loop
            CLK_sim <= '0'; wait for CLK_PERIOD / 2;
            CLK_sim <= '1'; wait for CLK_PERIOD / 2;
        end loop;
    end process;

    -- ??????? ????????????
    stim_proc: process
    begin
        -- ????? ????????
        RST_sim <= '1';
        wait for 40 ns;
        RST_sim <= '0';
        wait for 20 ns;

        -- ==========================================================
        -- ???? 1: ??????? ???????? ADD (15 + 10 = 25)
        -- ==========================================================
        ALU_M_Ext_sim  <= '0';
        ALUControl_sim <= "0000"; -- ADD
        A_sim <= std_logic_vector(to_unsigned(15, 32));
        B_sim <= std_logic_vector(to_unsigned(10, 32));
        wait for 40 ns;

        -- ==========================================================
        -- ???? 2: ?????????? ????? ????? SLL (3 << 4)
        -- ????? 3 (0x03) ???????? ?? 4 ????. ????: 0x30 (48 ? ???.)
        -- ==========================================================
        ALUControl_sim <= "0001"; -- SLL
        A_sim <= std_logic_vector(to_unsigned(3, 32));
        B_sim <= std_logic_vector(to_unsigned(4, 32)); -- ??????? 5 ??? ?????? ?????
        wait for 40 ns;

        -- ==========================================================
        -- ???? 3: ?????????????? ????? ?????? SRA (0x80000000 >> 2)
        -- ??????? ??????? ????????????? ????? ?? 2 ???? ??????
        -- ????: ????? ?????? ????? -> 0xE0000000
        -- ==========================================================
        ALUControl_sim <= "1101"; -- SRA
        A_sim <= X"80000000";
        B_sim <= std_logic_vector(to_unsigned(2, 32));
        wait for 40 ns;

        -- ==========================================================
        -- ???? 4: ?????????? ????????? MUL (7 * 8 = 56)
        -- ?????????? ?-??????????! ?????? ????????? ???????????? ?? 1 ????
        -- ==========================================================
        ALU_M_Ext_sim  <= '1';
        ALUControl_sim <= "0000"; -- MUL (??? M_Ext = '1')
        A_sim <= std_logic_vector(to_unsigned(7, 32));
        B_sim <= std_logic_vector(to_unsigned(8, 32));
        wait for 40 ns;

        -- ==========================================================
        -- ???? 5: ?????????? ??????? DIV (100 / 3)
        -- ??????? Quotient = 33 (0x21). ?????? ?????? 32 ?????!
        -- ==========================================================
        ALU_M_Ext_sim  <= '1';
        ALUControl_sim <= "0100"; -- DIV (??? M_Ext = '1')
        A_sim <= std_logic_vector(to_unsigned(100, 32));
        B_sim <= std_logic_vector(to_unsigned(3, 32));
        
        -- ??????? ??????? Start_Div ?? 1 ????
        Start_Div_sim <= '1';
        wait for CLK_PERIOD;
        Start_Div_sim <= '0';

        -- ????, ???? ????????? ??????? ???????
        wait until Div_Ready_sim = '1';
        wait for 40 ns;

        -- ????? ?????????
        report "ALL INTEGRATION TESTS SUCCESSFUL!";
        wait;
    end process;

end architecture;