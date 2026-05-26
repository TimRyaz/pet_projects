#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

struct Sensors {
	bool Start = false;
	bool Reset = false;
	bool DN = true;
	bool DP[4] = { false, false, false, false};
	bool DV[3] = { false };
	bool DR[3] = { false };

	bool D[8] = { false };

	bool BK0 = false;
	bool BK1 = false;

	bool DKR = false;
	bool DKL = false;

	bool AT = false;
	bool DT_IN = false;
	bool DT_OUT = false;

};
struct Controls {
	bool ML = false;
	bool MR = false;
	bool STOP = false;	
	bool V[3] = { false };

	bool MKR = false;
	bool MKL = false;
	void reset() {
		ML = MR = STOP = MKL = MKR = false;
		V[0] = V[1] = V[2] = false;
	}
	void print() {
		//if (ML) {cout << "[>>] ДВИЖЕНИЕ: Контейнер едет ВЛЕВО" << endl;}
		//else if (MR) {cout << "[>>] ДВИЖЕНИЕ: Контейнер едет ВПРАВО" << endl;}	
		//else {cout << "[--] ДВИЖЕНИЕ: Контейнер СТОИТ на месте" << endl;}

		// Логика поворота
		if (MKL) {
			cout << "Поворот контейнера: ВЛЕВО" << endl;
		}
		else if (MKR) {
			cout << "Поворот контейнера: ВПРАВО" << endl;
		}
		if (STOP) { cout << "Аварийная остановка:пересечение допустимой границы!\n";}
		if (V[0]) { cout << "Клапан резервуара R1 открыт\n"; }
		if (!(V[0])) { cout << "Клапан резервуара R1 закрыт\n"; }
		if (V[1]) { cout << "Клапан резервуара R2 открыт\n"; }
		if (!(V[1])) { cout << "Клапан резервуара R2 закрыт\n"; }
		if (V[2]) { cout << "Клапан резервуара R3 открыт\n"; }
		if (!(V[2])) { cout << "Клапан резервуара R3 закрыт\n"; }

	}
};
class MachineContext;
class State {
public:
	virtual ~State() = default;
	virtual void enter(MachineContext* ctx) {}
	virtual void execute(MachineContext* ctx) = 0;
	virtual void exit(MachineContext* ctx) {}
	virtual string getName() const = 0;
};

class IdleState;
class AlarmState;
class CheckNormState;
class MovingToR0LeftState;
class MovingToR1LefttState;
class MovingToR1RightState;
class MovingToR2LefttState;
class MovingToR2RightState;
class FillingFromR1;
class FillingFromR2;
class FillingFromR3;
class RubbishDump;
class Bunker2_Dump;
class Bunker5_Dump;


class MachineContext {
public:
	Sensors sensors;
	Controls controls;
	unique_ptr<State> currentState;

	void changeState(State* newState);
	void update();
};


class AlarmState :public State {
public:
	void enter(MachineContext* ctx) override {
		ctx->controls.reset();
		ctx->controls.STOP = true;
	}
	void execute(MachineContext* ctx) override;
	string getName() const override { return "ALARM"; }

};

class CheckNormState :public State {
	State* nextState;
public:
	CheckNormState(State* next) : nextState(next) {}

	void execute(MachineContext* ctx) override {
		if (ctx->sensors.DN) {
			ctx->changeState(nextState);		//если контейнер горизонтален, то переходим в следующее состояние, если нет, то переходим в аварию
		}
		else {
			ctx->changeState(new AlarmState());
			return;

		}
	}
	string getName() const override { return "CheckNorm"; }
};

class MovingToR0LeftState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR0LeftState(State* next) : nextState(next) {}		//сюда можно передавать как переход в ожидание, так и переход в сброс мусора

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.ML = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[0]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.ML = false;
			cout << "Прибытие на позицию DP0";
			ctx->changeState(new CheckNormState(nextState));	//успешное выполнение группы S_mp0L
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R0_Left"; }
};

class MovingToR1LeftState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR1LeftState(State* next) : nextState(next) {}		//сюда можно передавать как сброс в бункер B1B6, так и получение материала из R1

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.ML = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[1]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.ML = false;
			cout << "Прибытие на позицию DP1";
			ctx->changeState(new CheckNormState(nextState));	//успешное выполнение группы S_mp1L
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R1_Left"; }
};

class MovingToR1RightState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR1RightState(State* next): nextState(next){}		//сюда можно передавать как сброс в бункер B1B6, так и получение материала из R1

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.MR = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[1]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.MR = false;
			cout << "Прибытие на позицию DP1";
			ctx->changeState(new CheckNormState(nextState));	//успешное выполнение группы S_mp1r
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R1_Right"; }
};

class MovingToR2LeftState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR2LeftState(State* next) : nextState(next) {}		//сюда можно передавать как сброс в бункер B1B6, так и получение материала из R1

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.ML = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[2]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.ML = false;
			cout << "Прибытие на позицию DP2";
			ctx->changeState(new CheckNormState(nextState));	//успешное выполнение группы S_mp2L
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R2_Left"; }
};

class MovingToR2RightState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR2RightState(State* next) : nextState(next) {}		//сюда можно передавать как сброс в бункер B1B6, так и получение материала из R1

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.MR = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[2]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.MR = false;
			cout << "Прибытие на позицию DP2";
			ctx->changeState(new CheckNormState(nextState));	//успешное выполнение группы S_mp2r
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R2_Right"; }
};

class MovingToR3LeftState :public State {
	int SubState = 1;
	State* nextState;
public:
	MovingToR3LeftState(State* next) : nextState(next) {}		//сюда можно передавать как сброс в бункер B1B6, так и получение материала из R1

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.MR = true;
			if (!ctx->sensors.DN) {
				cout << "[!] Аварийная ситуация! Контейнер накренился во время передвижения!";
				ctx->changeState(new AlarmState());
				nextState = nullptr;
				break;
			}
			if (ctx->sensors.DP[3]) {
				SubState = 2;
			}
		case 2:
			ctx->controls.MR = false;
			cout << "Прибытие на позицию DP3";
			ctx->changeState(new CheckNormState(nextState)); //успешное выполнение группы S_mp3r (здесь названа по-другому)
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Move_to_R3_Left"; }
};

class Bunker2_Dump :public State {
	int SubState = 1;
	State* nextState;

public:
	Bunker2_Dump(State* next) : nextState(next) {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			if (!ctx->sensors.D[2]) {	//если контейнер B2 не полон, то переходим в S_b22
				SubState = 3;
			}
			else {
				SubState = 2;				//Переходим в S_Idle_Drain_B2
			}
			break;
		case 2:
			if (!ctx->sensors.D[2]) {
				SubState = 3;				//бункер B2 опустошен, сливаем в бункер
			}
			break;
		case 3:
			ctx->controls.MKL = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			if (ctx->sensors.DKL) {
				SubState = 4;
			}
			break;
		case 4:
			ctx->controls.MKL = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			SubState = 5;
			break;
		case 5:
			if (ctx->sensors.DT_OUT) {
				SubState = 6;
			}
			break;
		case 6:	
			ctx->controls.MKR = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			if (ctx->sensors.DN) {
				SubState = 7;
			}
			break;
		case 7:
			ctx->controls.MKR = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			ctx->changeState(nextState);
			nextState = nullptr;
			break;
		}
	}
	string getName() const override { return "Bunker2_Dump"; }
};

class Bunker5_Dump :public State {
	int SubState = 1;
	State* nextState;

public:
	Bunker5_Dump(State* next) : nextState(next) {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			if (!ctx->sensors.D[5]) {	//если контейнер B0 не полон, то переходим в S_Rub2
				SubState = 3;
			}
			else {
				SubState = 2;				//Переходим в S_Idle_Drain_Rub

			}
			break;
		case 2:
			if (!ctx->sensors.D[5]) {
				SubState = 3;				//бункер B0 опустошен, сливаем в бункер
			}
			break;
		case 3:
			ctx->controls.MKR = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			if (ctx->sensors.DKR) {
				SubState = 4;
			}
			break;
		case 4:
			ctx->controls.MKR = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			SubState = 5;
			break;
		case 5:
			if (ctx->sensors.DT_OUT) {
				SubState = 6;
			}
			break;
		case 6:
			ctx->controls.MKL = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			if (ctx->sensors.DN) {
				SubState = 7;
			}
			break;
		case 7:
			ctx->controls.MKL = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			ctx->changeState(nextState);
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Bunker5_Dump"; }
};

class RubbishDump :public State {
	int SubState = 1;
	State* nextState;

public:
	RubbishDump(State* next) : nextState(next) {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			if (!ctx->sensors.D[0]) {	//если контейнер B0 не полон, то переходим в S_Rub2
				SubState = 3;
			}
			else {
				SubState = 2;				//Переходим в S_Idle_Drain_Rub

			}
			break;
		case 2:
			if (!ctx->sensors.D[0]) {
				SubState = 3;				//бункер B0 опустошен, сливаем в бункер

			}
			break;
		case 3:
			ctx->controls.MKL = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			if (ctx->sensors.DKL) {
				SubState = 4;
			}
			break;
		case 4:
			ctx->controls.MKL = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKR = false;
			SubState = 5;
			break;
		case 5:
			if (ctx->sensors.DT_OUT) {
				SubState = 6;

			}
			break;
		case 6:
			ctx->controls.MKR = true;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			if (ctx->sensors.DN) {
				SubState = 7;
			}
			break;
		case 7:
			ctx->controls.MKR = false;		//однозначно задаем состояния двигателей поворота для надежности
			ctx->controls.MKL = false;
			ctx->changeState(nextState);
			nextState = nullptr;
			break;
		}

	}
	string getName() const override { return "Rubbish_Dump"; }
};

class FillingFromR1 :public State {
private:
	int SubState = 1;
	State* nextState; // Владение этим указателем передается от IdleState

public:
	FillingFromR1(State* next) :nextState(next) {}

	//~FillingFromR1() override {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			// Включаем клапан
			ctx->controls.V[0] = true;
			if (ctx->sensors.DV[0]) {
				SubState = 2;
			}
			break;
		case 2:
			ctx->controls.V[0] = true;
			if (ctx->sensors.DT_IN) {
				ctx->controls.V[0] = false;		//сразу выключаем для надежности
				SubState = 3;
			}
			if (ctx->sensors.DR[0]) {
				ctx->controls.V[0] = false;
				ctx->changeState(new RubbishDump(nextState));
				ctx->controls.V[0] = false;
				nextState = nullptr; // Передаем владение RubbishDump
				break;
			}
			break;
		case 3:
			// Выключаем клапан
			ctx->controls.V[0] = false;

			if (ctx->sensors.AT) {
				if (nextState != nullptr) {
					nextState = nullptr;
				}
				ctx->changeState(new AlarmState());
				break;
			}

			// Ждем закрытия клапана и меняем состояние
			if (!ctx->sensors.DV[0]) {
				ctx->changeState(nextState);
				nextState = nullptr;
			}
			break;
		}
	}
	string getName() const override { return "Filling_From_R1"; }
};

class FillingFromR2 :public State {
	int SubState = 1;
	State* nextState;
public:
	FillingFromR2(State* next) :nextState(next) {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.V[1] = true;
			if (ctx->sensors.DV[1]) {
				SubState = 2;
			}
			break;
		case 2:
			ctx->controls.V[1] = true;
			if (ctx->sensors.DT_IN) {
				ctx->controls.V[1] = false;		//сразу выключаем для надежности
				SubState = 3;	
			}
			if (ctx->sensors.DR[1]) {
				ctx->controls.V[1] = false;
				ctx->changeState(new RubbishDump(nextState));	//переход в сброс мусора если резервуары пусты
				nextState = nullptr;
				break;
			}
			break;
		case 3:
			ctx->controls.V[1] = false;
			if (ctx->sensors.AT) {
				if (nextState != nullptr) {
					nextState = nullptr;
				}
				ctx->changeState(new AlarmState());
				break;
			}
			if (!ctx->sensors.DV[1]) {
				ctx->changeState(nextState);
				nextState = nullptr;
			}
			break;
		}
	}
	string getName() const override { return "Filling_From_R2"; }
};

class FillingFromR3 :public State {
	int SubState = 1;
	State* nextState;
public:
	FillingFromR3(State* next) :nextState(next) {}

	void execute(MachineContext* ctx) override {
		switch (SubState) {
		case 1:
			ctx->controls.V[2] = true;
			if (ctx->sensors.DV[2]) {
				SubState = 2;
			}
			break;
		case 2:
			ctx->controls.V[2] = true;
			if (ctx->sensors.DT_IN) {
				ctx->controls.V[2] = false;		//сразу выключаем клапан R3
				SubState = 3;
			}
			if (ctx->sensors.DR[2]) {
				ctx->controls.V[2] = false;
				ctx->changeState(new RubbishDump(nextState));	//переход в сброс мусора, если R3 опусутошится
				nextState = nullptr;
				break;
			}
			break;
		case 3:
			ctx->controls.V[2] = false;
			if (ctx->sensors.AT) {
				if (nextState != nullptr) {
					nextState = nullptr;
				}
				ctx->changeState(new AlarmState());
			}
			if (!ctx->sensors.DV[2]) {
				ctx->changeState(nextState);
				nextState = nullptr;
			}
			break;
		}
	}
	string getName() const override { return "Filling_From_R3"; }
};

class IdleState : public State {
public:
	void execute(MachineContext* ctx) override {
		if (ctx->sensors.DN && ctx->sensors.Start && ctx->sensors.DP[0]) {
			cout << "Запуск цикла..." << endl;

			// СТРОИМ ЦЕПОЧКУ В ОБРАТНОМ ПОРЯДКЕ (как матрешку):
			// 
			//12. Возвращаемся в изначальное состояние (DP0)
			State* step12 = new MovingToR0LeftState(new IdleState());

			//11. Сбрасываем в B5
			State* step11 = new Bunker5_Dump(step12);
			
			//10. Едем к B5 
			State* step10 = new MovingToR2LeftState(step11);
			
			//9. Наполняем контейнер материалом из R3
			State* step9 = new FillingFromR3(step10);
			
			//8. Едем к R3 за материалом 
			State* step8 = new MovingToR3LeftState(step9);

			//7. Не отъезжая, наполняем контейнер материалом из R2
			State* step7 = new FillingFromR2(step8);
			
			//6. Сбрасываем материал в B2
			State* step6 = new Bunker2_Dump(step7);
			
			//5. Едем к B2 сбрасывать материал в B2
			State* step5 = new MovingToR2LeftState(step6);
			
			//4. Наполняем контейнер материалом из R3
			State* step4 = new FillingFromR3(step5);

			// 3. Едем к R3 за материалом
			State* step3 = new MovingToR3LeftState(step4);

			// 2. Перед отъездом нужно наполниться из R1
			State* step2 = new FillingFromR1(step3);

			// 1. Первое действие: Ехать к R1 (вправо). Передаем ему шаг 2.
			State* step1 = new MovingToR1RightState(step2);

			ctx->changeState(step1);
		}
	}
	string getName() const override { return "Idle"; }
};

void AlarmState::execute(MachineContext* ctx) {
	if (ctx->sensors.Reset) {
		cout << "Выход из аварии. Переход в состояние ожидания";
		ctx->changeState(new IdleState());
	}
}

void MachineContext::changeState(State* newState) {
	if (currentState) {
		currentState->exit(this);
	}
	// Используем reset для сырого указателя и передачи владения в unique_ptr
	currentState.reset(newState);

	if (currentState) {
		cout << "Переход в состояние: " << currentState->getName() << "\n";
		currentState->enter(this);
	}
}

void MachineContext::update() {
	if (currentState) {
		if (sensors.AT || sensors.BK0 || sensors.BK1) {
			if (currentState->getName() != "ALARM") {
				cout << "ОБНАРУЖЕНА АВАРИЙНАЯ СИТУАЦИЯ В АСУ!";
				changeState(new AlarmState());
				return;
			}
		}
		currentState->execute(this);
		controls.print();
		cout << endl;
	}
}

void SimulatePhysics(MachineContext& mc, const string& message) {
	cout << "Симуляция:" << message<<"\n";
	this_thread::sleep_for(chrono::milliseconds(1000)); //симулируем ожидание для какого-то действия
	mc.update();
}

int main() {
	setlocale(LC_ALL, "Russian");
	MachineContext mc;

	// 1. Инициализация
	mc.changeState(new IdleState());

	// ЭТАП 1: ЗАПУСК
	mc.sensors.Start = true;
	mc.sensors.DP[0] = true;
	SimulatePhysics(mc, "Оператор нажимает кнопку START. Контейнер на базе (DP0).");

	// ЭТАП 2: ДВИЖЕНИЕ К R1 (ВПРАВО)
	mc.sensors.DP[0] = false;
	SimulatePhysics(mc, "Контейнер поехал вправо... (DP0=0)");

	mc.sensors.DP[1] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли к датчику DP1. Проверка горизонта (DN=1).");
	// Переход: MovingToR1Right -> CheckNorm -> FillingFromR1

	// ЭТАП 3: НАПОЛНЕНИЕ ИЗ R1 
	// 1. Клапан открывается
	mc.sensors.DV[0] = true; // Сначала ставим датчик
	SimulatePhysics(mc, "Клапан R1 открылся (DV[0]=1).");
	// Update видит DV[0]=true -> переходит к ожиданию наполнения

	// 2. Бак наполняется
	mc.sensors.DT_IN = true; // Сначала ставим датчик уровня
	SimulatePhysics(mc, "Бак наполнился! (DT_IN=1).");
	// Update видит DT_IN=1 -> дает команду закрыть клапан (переход в Case 3)

	// 3. Клапан закрывается
	mc.sensors.DV[0] = false; // Физическое закрытие
	mc.sensors.DT_IN = false;
	SimulatePhysics(mc, "Клапан R1 закрылся (DV[0]=0).");
	// Update видит Case 3 -> Выключает V[0] -> Проверяет DV[0]=0 -> Переходит дальше
	// 
	// ЭТАП 4: ДВИЖЕНИЕ К R3 (ВПРАВО)
	mc.sensors.DP[1] = false;
	SimulatePhysics(mc, "Контейнер уехал с DP1 к R3...");

	mc.sensors.DP[3] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли к датчику DP3. Проверка DN.");

	// ЭТАП 5: НАПОЛНЕНИЕ ИЗ R3
	mc.sensors.DV[2] = true;
	SimulatePhysics(mc, "Клапан R3 открылся (DV[2]=1).");

	mc.sensors.DT_IN = true;
	SimulatePhysics(mc, "Бак наполнился (DT_IN=1).");

	mc.sensors.DV[2] = false;
	mc.sensors.DT_IN = false;
	SimulatePhysics(mc, "Клапан R3 закрылся (DV[2]=0).");


	// ЭТАП 6: ДВИЖЕНИЕ К R2 (ВЛЕВО)
	mc.sensors.DP[3] = false;
	SimulatePhysics(mc, "Контейнер едет влево к R2...");

	mc.sensors.DP[2] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли к датчику DP2. Проверка DN.");

	// ЭТАП 7: СБРОС В БУНКЕР 2
	mc.sensors.D[2] = false;
	SimulatePhysics(mc, "Проверка бункера: он пуст (D[2]=0). Начинаем сброс.");

	mc.sensors.DKL = true;
	mc.sensors.DN = false;
	SimulatePhysics(mc, "Контейнер наклонился влево (DKL=1). DN=0.");
	SimulatePhysics(mc, "Выключение мотора наклона");

	mc.sensors.DT_OUT = true;
	cout << "[Визуализация] Материал высыпается..." << endl;
	// Тут можно сделать доп. update, пока сыпется
	SimulatePhysics(mc, "Датчик пролета материала сработал (DT_OUT=1)");

	// Возврат в исходное
	mc.sensors.DT_OUT = false; // Мусор высыпался
	SimulatePhysics(mc, "Начало возврата в горизонт (DN=0)");
	mc.sensors.DKL = false;    // Больше не наклонен

	mc.sensors.DN = true;      // Вернулся в горизонт
	SimulatePhysics(mc, "Контейнер вернулся в горизонт (DN=1).");

	// ЭТАП 8: НАПОЛНЕНИЕ ИЗ R2
	mc.sensors.DV[1] = true;
	SimulatePhysics(mc, "Клапан R2 открылся (DV[1]=1).");

	mc.sensors.DT_IN = true;
	SimulatePhysics(mc, "Бак наполнился (DT_IN=1).");

	mc.sensors.DV[1] = false;
	SimulatePhysics(mc, "Клапан R2 закрыт, ожидаем фи");
	mc.sensors.DT_IN = false;
	SimulatePhysics(mc, "Клапан R2 закрылся (DV[1]=0).");

	// ЭТАП 9: ДВИЖЕНИЕ К R3 (СНОВА)
	mc.sensors.DP[2] = false;
	SimulatePhysics(mc, "Контейнер едет к R3...");

	mc.sensors.DP[3] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли на DP3. Проверка DN.");

	// ЭТАП 10: НАПОЛНЕНИЕ ИЗ R3 (ПОВТОРНО)
	mc.sensors.DV[2] = true;
	SimulatePhysics(mc, "Клапан R3 открылся (DV[2]=1).");

	mc.sensors.DT_IN = true;
	SimulatePhysics(mc, "Бак наполнился (DT_IN=1).");

	mc.sensors.DV[2] = false;
	mc.sensors.DT_IN = false;
	SimulatePhysics(mc, "Клапан R3 закрылся (DV[2]=0).");

	// ЭТАП 11: ВОЗВРАТ К R2 И СБРОС В B5
	mc.sensors.DP[3] = false;
	SimulatePhysics(mc, "Едем обратно к R2...");

	mc.sensors.DP[2] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли на DP2. Переход к сбросу в B5.");

	// Логика сброса в Bunker 5
	mc.sensors.D[5] = false; // Бункер 5 пуст
	SimulatePhysics(mc, "Проверка B5. Пуст. Начинаем сброс.");

	mc.sensors.DKR = true; // Наклон ВПРАВО
	mc.sensors.DN = false;
	SimulatePhysics(mc, "Контейнер наклонился вправо (DKR=1).");
	SimulatePhysics(mc, "Выключение мотора наклона");

	mc.sensors.DT_OUT = true;
	SimulatePhysics(mc, "Материал высыпался (DT_OUT=1).");

	// Возврат
	mc.sensors.DT_OUT = false;
	SimulatePhysics(mc, "Начало возврата в горизонт");
	mc.sensors.DKR = false;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Контейнер вернулся в горизонт (DN=1).");

	// ЭТАП 12: ВОЗВРАТ НА БАЗУ R0
	mc.sensors.DP[2] = false;
	SimulatePhysics(mc, "Финишная прямая: едем на базу R0...");

	mc.sensors.DP[0] = true;
	mc.sensors.DN = true;
	SimulatePhysics(mc, "Прибыли на базу DP0. Цикл завершен.");

	cout << "ТЕХНОЛОГИЧЕСКИЙ ЦИКЛ ЗАВЕРШЕН!" << endl;
	mc.sensors.Start = false;
	return 0;
}