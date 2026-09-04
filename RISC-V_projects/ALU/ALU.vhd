library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ALU is
    port(
        CLK        : in  STD_LOGIC;
        RST        : in  STD_LOGIC;
        Start_Div  : in  STD_LOGIC;
        A          : in  STD_LOGIC_VECTOR(31 downto 0);
        B          : in  STD_LOGIC_VECTOR(31 downto 0);
        ALUControl : in  STD_LOGIC_VECTOR(3 downto 0);
        ALU_M_Ext  : in  STD_LOGIC;
        Div_Ready  : out STD_LOGIC;
        Result     : out STD_LOGIC_VECTOR(31 downto 0)
    );
end entity;

architecture synth of ALU is
    signal Sum          : STD_LOGIC_VECTOR(31 downto 0);
    signal ch_mux2      : STD_LOGIC_VECTOR(31 downto 0);
    signal Cout         : STD_LOGIC;
    signal res_internal : STD_LOGIC_VECTOR(31 downto 0);
    signal Overflow     : STD_LOGIC;
    signal is_sub       : STD_LOGIC;

    signal Shifter_Res  : STD_LOGIC_VECTOR(31 downto 0);
    signal ShiftCtrl    : STD_LOGIC_VECTOR(1 downto 0);
    
    signal Div_Q        : STD_LOGIC_VECTOR(31 downto 0);
    signal Div_R        : STD_LOGIC_VECTOR(31 downto 0);
    -- ?????????? ?????? ??? ?????? ????? ?????????? ???????
    signal div_ready_int: STD_LOGIC; 

    signal Mul_64       : unsigned(63 downto 0);

begin
    -- ??????????: ????????? ??????? 4-???????
    is_sub <= '1' when (ALUControl = "1000" or ALUControl = "0010" or ALUControl = "0011") else '0';
    ch_mux2 <= not B when is_sub = '1' else B;

    summ: entity work.summ_kogge_stone 
        port map (
            A    => A,
            B    => ch_mux2,
            Cin  => is_sub, -- ??????? ?? ????? ????? 1 ??? ????????? (?????????? ?? 2)
            Sum  => Sum,
            Cout => Cout
        );

    Overflow <= (A(31) xor Sum(31)) and (is_sub xnor A(31) xnor B(31)) and not ALUControl(1);
    
    ShiftCtrl <= "00" when ALUControl = "0001" else
                 "01" when ALUControl = "0101" else 
                 "10" when ALUControl = "1101" else "00";

    shifter_inst: entity work.alu_shifter 
        port map(
            A         => A,
            Shamt     => B(4 downto 0),
            ShiftCtrl => ShiftCtrl,
            Result    => Shifter_Res
        );
    
    divider_inst: entity work.divider 
        port map(
            CLK   => CLK,
            RST   => RST,
            Start => Start_Div,
            A     => A,
            B     => B,
            Q     => Div_Q,
            R     => Div_R,
            Ready => div_ready_int -- ????? ?? ?????????? ??????
        );

    Mul_64 <= unsigned(A) * unsigned(B);

    -- ??????? ??????????
    process (A, B, ALUControl, ALU_M_Ext, Sum, Shifter_Res, Div_Q, Div_R, Mul_64, Overflow, div_ready_int) 
    begin
        if ALU_M_Ext = '1' then
            case ALUControl(2 downto 0) is
                when "000" => res_internal <= std_logic_vector(Mul_64(31 downto 0));
                when "100" => 
                    if div_ready_int = '1' then 
                        res_internal <= Div_Q;
                    else 
                        res_internal <= (others => '0');
                    end if;
                when "110" => 
                    if div_ready_int = '1' then 
                        res_internal <= Div_R;
                    else 
                        res_internal <= (others => '0');
                    end if;
                when others => res_internal <= X"00000000";
            end case;
        else
            case ALUControl is
                when "0000" => res_internal <= Sum;          -- ADD
                when "1000" => res_internal <= Sum;          -- SUB
                when "0111" => res_internal <= A and B;       -- AND
                when "0110" => res_internal <= A or B;        -- OR
                when "0010" => res_internal <= (0 => (Sum(31) xor Overflow), others => '0'); -- SLT
                when "0011" => res_internal <= (0 => Cout, others => '0'); -- SLTU
                when "0001" | "0101" | "1101" => res_internal <= Shifter_Res; -- ??????????: ?????? ?????!
                when others => res_internal <= X"00000000";
            end case;
        end if;
    end process;

    -- ????????? ?????? ??????
    Result    <= res_internal;
    Div_Ready <= div_ready_int;

end architecture;
