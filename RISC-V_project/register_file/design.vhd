library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD_UNSIGNED.all;

entity register is
	port(clk, we3: in STD_LOGIC;
		a1, a2, a3: in STD_LOGIC_VECTOR(5 downto 0);
		wd3: in STD_LOGIC_VECTOR(31 downto 0);
		rd1, rd2: out STD_LOGIC_VECTOR(31 downto 0));
end entity;

architecture synth of register is
	type ramtype us array(31 downto 0) of STD_LOGIC_VECTOR(31 downto 0);
	signal memory: ramtype;
begin
	--трехпортовый регистровый файл
	--комбинационное чтение двух портов (A1/RD1, A2/RD2)
	--запись в третий порт по переднему фронту тактового импульса
	--значение нулевого регистра жестко связано с 0
	process(clk) begin
		if rising_edge(clk) then
			if we3 = '1' then memory(to_integer(a1)) <= wd3;
			end if;
		end if;
	end process;
	process(a1, a2) begin
		if (to_integer(a1) = 0) then rd1 <= X"00000000"; --нулевой регистра
		else rd1 <= memory(to_integer(a1));
		end if;
		if (to_integer(a2) = 0)then rd2 <= X"00000000";
		else rd2 <= memory(to_integer(a2));
		end if;
	end process;
end architecture;
