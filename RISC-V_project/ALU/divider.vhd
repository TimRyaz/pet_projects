library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity divider is
	port(
	CLK: in STD_LOGIC;
	RST: in STD_LOGIC;
	Start: in STD_LOGIC;
	A: in STD_LOGIC_VECTOR(31 downto 0);
	B: in STD_LOGIC_VECTOR(31 downto 0);
	Q: out STD_LOGIC_VECTOR(31 downto 0);
	R: out STD_LOGIC_VECTOR(31 downto 0);
	Ready: out STD_LOGIC);
end entity;

architecture synth of divider is
	type statetype is (IDLE, DIVIDING, DONE);
	signal state: statetype := IDLE;
	
	signal B_Reg : unsigned(31 downto 0) := (others => '0');
	signal shift_reg : unsigned(63 downto 0) := (others => '0');
	signal counter: integer range 0 to 32 := 0;
begin
	process(CLK, RST) 
		variable next_shift: unsigned(63 downto 0);
	begin
		if RST = '1' then
			state <= IDLE;
			B_reg <= (others => '0');
			shift_reg <= (others => '0');
			counter <= 0;
			Ready <= '0';
		elsif rising_edge(clk) then
			case state is
				when IDLE =>
					Ready <= '0';
					if Start = '1' then
						B_reg<= unsigned(B);
						shift_reg <= unsigned(X"00000000" & A);
						counter <= 32;
						state <= DIVIDING;
					end if;
				when DIVIDING =>
					next_shift := shift_reg(62 downto 0) & '0';
					if next_shift(63 downto 32) >= B_reg then
						shift_reg(63 downto 32) <= next_shift(63 downto 32) - B_reg;
						shift_reg(31 downto 1) <= next_shift(31 downto 1);
						shift_reg(0) <= '1';
					else
						shift_reg <= next_shift;
						shift_reg(0) <= '0';
					end if;
					if counter = 1 then
						state <= DONE;
					else 
						counter <= counter - 1;
					end if;
				when DONE =>
					Ready <= '1';
					state <= IDLE;
				when others => 
					state <= IDLE;
			end case;
		end if;
	end process;
	Q <= std_logic_vector(shift_reg(31 downto 0));
	R <= std_logic_vector(shift_reg(63 downto 32));
end architecture;

