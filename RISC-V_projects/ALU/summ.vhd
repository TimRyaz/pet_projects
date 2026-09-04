library IEEE;
use IEEE.STD_LOGIC_1164.all;

entity summ_kogge_stone is
	port(A, B : in STD_LOGIC_VECTOR(31 downto 0);
	Cin : in STD_LOGIC;
	Sum: out STD_LOGIC_VECTOR(31 downto 0);
	Cout: out STD_LOGIC);
end entity;

architecture synth of summ_kogge_stone is
	type level_signal is array (31 downto 0) of STD_LOGIC;
	signal p0, g0: level_signal; 
	signal p1, g1: level_signal; 
	signal p2, g2: level_signal; 
	signal p3, g3: level_signal; 
	signal p4, g4: level_signal; 
	signal p5, g5: level_signal; 

	signal C: STD_LOGIC_VECTOR(32 downto 0);
begin
	-- ?????? 0: ?????????? ??????? P ? G. 
	-- ? ????? ??????? ?????? G ??????????? Cin, ????? ????? ??? ????!
	p0(0) <= A(0) xor B(0);
	g0(0) <= (A(0) and B(0)) or ((A(0) xor B(0)) and Cin);

	gen_pg0: for i in 1 to 31 generate 
		p0(i) <= A(i) xor B(i);
		g0(i) <= A(i) and B(i);
	end generate;

	-- ?????? 1 (????? 1)
	p1(0) <= p0(0); g1(0) <= g0(0);
	gen_pg1: for i in 1 to 31 generate	
		g1(i) <= g0(i) or (p0(i) and g0(i-1));
		p1(i) <= p0(i) and p0(i-1);
	end generate;

	-- ?????? 2 (????? 2)
	p2(1 downto 0) <= p1 (1 downto 0);
	g2(1 downto 0) <= g1 (1 downto 0);
	gen_pg2: for i in 2 to 31 generate
		g2(i) <= g1(i) or (p1(i) and g1(i-2));
		p2(i) <= p1(i) and p1(i-2);
	end generate;

	-- ?????? 3 (????? 4)
	p3(3 downto 0) <= p2 (3 downto 0);
	g3(3 downto 0) <= g2(3 downto 0);
	gen_pg3: for i in 4 to 31 generate
		g3(i) <= g2(i) or (p2(i) and g2(i-4));
		p3(i) <= p2(i) and p2(i-4);
	end generate;

	-- ?????? 4 (????? 8)
	p4(7 downto 0) <= p3 (7 downto 0);
	g4(7 downto 0) <= g3 (7 downto 0);
	gen_pg4: for i in 8 to 31 generate
		g4(i) <= g3(i) or (p3(i) and g3(i-8));
		p4(i) <= p3(i) and p3(i-8);
	end generate;

	-- ?????? 5 (????? 16)
	p5(15 downto 0) <= p4(15 downto 0);
	g5(15 downto 0) <= g4(15 downto 0);
	gen_pg5: for i in 16 to 31 generate
		g5(i) <= g4(i) or (p4(i) and g4(i-16));
		p5(i) <= p4(i) and p4(i-16);
	end generate;
	
	-- ?????? g5 ???????? ????????? ??? ?????????? ????????!
	C(0) <= Cin;
	gen_carries: for i in 0 to 31 generate
		C(i+1) <= g5(i);
	end generate;

	gen_sum: for i in 0 to 31 generate
		Sum(i) <= (A(i) xor B(i)) xor C(i); -- ???????? ? ?????? XOR ?????????
	end generate;

	Cout <= C(32);
end architecture;
