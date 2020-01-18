#include "main.h"
using namespace okapi;
struct PID
{
	float kP;
	float kI;
	float kD;
	float integral;
	float derivative;
	float error;
	float previous_error;
	float speed;
	float target;
	float sensor;
};
typedef struct PID pid;
pid LT;
pid TL;
//others

pros::ADIPotentiometer trayPot('H');
pros::ADIPotentiometer liftPot('F');
//pros::ADIEncoder trackingWheel('B', 'C');
//Inertia sensor: port 13
okapi::Controller controller;
okapi::Motor trayLift(-16);
okapi::Motor armLift(9);
okapi::MotorGroup rollers({-5, 8});
okapi::Motor rollerOne(-5);
okapi::Motor rollerTwo(8); 
int lcdCounter = 1;
int buttonCount = 0;
bool isPressed = false;
double slowTraySpeed = 27.5; 
double fastTraySpeed = 200;
bool holdTray = false;
bool holdTray2 = false;
int trayPosition = 400;
bool holdLift = false;
double slowMoveKP = 0.0005;//0.001
double fastMoveKP = 0.002;
int holdToggle = 0; 
auto chassis = okapi::ChassisControllerFactory::create({20, 19}, {-11, -3}, okapi::AbstractMotor::gearset::green, {4.125, 10});

void trayTask(void *param);
void trayLiftTask(void *param);
//void armTask(void *param);
//backLiftPID(400);
//auto motorGroup = okapi::ChassisControllerFactory::create({3,-10}, okapi::AbstractMotor::gearset::green,{4.125,10});
//NEED PORT auto lift = okapi::ChassisControllerFactory::create()
//Position Tracking start
//float dTheta; //= (renc - lenc)/chaswidth -> radians
//float sector; //sector = (renc+lence)/2
//float radius; //radius = sector/dTheta
//End


void backLiftPID(double degrees)
{
	TL.target = degrees;
	TL.integral = 0;
	TL.sensor = trayLift.getPosition();
	TL.error = TL.target - TL.sensor;
	int timer = 0;

	while (abs(TL.error) >= 40)//(abs(TL.error) >= 40)
	{									   //or while(timer < 50){
		TL.kP = 0.5;					   //need tuning
		TL.kD = 0.2;					   //need tuning
		TL.kI = 0;						   //need tuning
		TL.sensor = trayLift.getPosition(); // = sensor.getValue || post setup
		TL.error = TL.target - TL.sensor;
		TL.derivative = TL.error - TL.previous_error;
		TL.integral += TL.error;
		TL.previous_error = TL.error;
		TL.speed = (TL.kP * TL.error + TL.kD * TL.derivative + TL.kI * TL.integral);
		trayLift.moveVelocity(TL.speed);
		//fill
		timer += 20;
		std::cout << "\nPot Value:" << trayLift.getPosition();
		pros::delay(20);
	}
}

void trayLiftPID(double value)
{
	TL.target = value;
	TL.integral = 0;
	TL.sensor = trayPot.get_value();
	TL.error = TL.target - TL.sensor;
	int timer = 0;

	while (true)//check GIT
	{									   //or while(timer < 50){
		TL.kP = 0.2;					   //need tuning
		TL.kD = 0.1;					   //need tuning
		TL.kI = 0;						   //need tuning
		TL.sensor = trayPot.get_value(); // = sensor.getValue || post setup
		TL.error = TL.target - TL.sensor;
		TL.derivative = TL.error - TL.previous_error;
		TL.integral += TL.error;
		TL.previous_error = TL.error;
		TL.speed = (TL.kP * TL.error + TL.kD * TL.derivative + TL.kI * TL.integral);
		trayLift.moveVelocity(TL.speed);
		//fill
		timer++;
		pros::delay(20);
		std::cout << "\nPot Value:" << trayPot.get_value();
	}

}

void armLiftPID(double degrees)
{
	LT.target = degrees;
	LT.integral = 0;
	LT.sensor = armLift.getPosition();
	LT.error = LT.target - LT.sensor;
	int timer = 0;

	while (abs(LT.error) >= 40)
	{									   //or while(timer < 50){
		LT.kP = 0.15;					   //need tuning
		LT.kD = 0.1;					   //need tuning
		LT.kI = 0;						   //need tuning
		LT.sensor = armLift.getPosition(); // = sensor.getValue || post setup
		LT.error = LT.target - LT.sensor;
		LT.derivative = LT.error - LT.previous_error;
		LT.integral += LT.error;
		LT.speed = (LT.kP * LT.error + LT.kD * LT.derivative + LT.kI * LT.integral);
		armLift.moveVelocity(LT.speed);
		//fill
		timer += 20;
		pros::delay(20);
		
	}
}

void movePID(double distanceL, double distanceR, double speedkP, int ms)
{
	double targetL = distanceL * 360 / (2 * 3.1415 * (4.125 / 2));
	double targetR = distanceR * 360 / (2 * 3.1415 * (4.125 / 2));
	auto drivePIDL = okapi::IterativeControllerFactory::posPID(speedkP, 0.001, 0.0015); //= data
	auto drivePIDR = okapi::IterativeControllerFactory::posPID(speedkP, 0.001, 0.0015);
	chassis.resetSensors();

	int timer = 0;
	double errorL;
	double errorR;
	double powerL;
	double powerR;

	while (timer < ms)
	{
		errorL = targetL - chassis.getSensorVals()[0];
		errorR = targetR - chassis.getSensorVals()[1];
		powerL = drivePIDL.step(errorL);
		powerR = drivePIDR.step(errorR);
		chassis.tank(-powerL, -powerR);

		pros::delay(10);
		timer += 10;
	}

	chassis.tank(0, 0);
}

void trayTask(void *){

	//TL.target = 800;

	while (true)
	{
		if (controller[ControllerDigital::right].changedToPressed())
		{
			holdTray = !holdTray;
			pros::delay(100);
		}
		if (holdTray){					//or while(timer < 50){
			//backLiftPID(400);
			//holdTray = !holdTray;
			//pros::delay(100);
		}
		else {
			trayLift.moveVelocity(slowTraySpeed * controller.getDigital(ControllerDigital::R1) +
				fastTraySpeed * controller.getDigital(ControllerDigital::left) - fastTraySpeed * controller.getDigital(ControllerDigital::R2));
		}
		pros::delay(25);
	}
}

void trayLiftTask(void *){
	while(true){
	std::cout << "\nPot Value:" << holdTray;
	TL.target = trayPosition;
	TL.integral = 0;
	TL.sensor = trayLift.getPosition();
	TL.error = TL.target - TL.sensor;
	int timer = 0;

	while (holdTray)//(abs(TL.error) >= 40)
	{									   //or while(timer < 50){
		TL.kP = 0.4;					   //need tuning
		TL.kD = 0;					   //need tuning
		TL.kI = 0;						   //need tuning
		TL.sensor = trayLift.getPosition(); // = sensor.getValue || post setup
		TL.error = TL.target - TL.sensor;
		TL.derivative = TL.error - TL.previous_error;
		TL.integral += TL.error;
		TL.previous_error = TL.error;
		TL.speed = (TL.kP * TL.error + TL.kD * TL.derivative + TL.kI * TL.integral);
		trayLift.moveVelocity(TL.speed);
		//fill
		timer += 20;
		//std::cout << "\nPot Value:" << holdTray;
		pros::delay(20);
	}
	}
}

void armTask(void *){
	
	//LT.target = 400;
	
	while (true){
		if (controller[ControllerDigital::A].changedToPressed())
		{
			holdLift = !holdLift;
		}

		if (holdLift)
		{
			armLiftPID(400);
		}
		else {
			armLift.controllerSet(controller.getDigital(ControllerDigital::X) - controller.getDigital(ControllerDigital::B));
		}
		
		pros::delay(25);
	}
}

void opcontrol()
{
	pros::Task trayLiftTaskHandle(trayLiftTask);
	pros::Task trayTaskHandle(trayTask);
	pros::Task armTaskHandle(armTask);
	
	 while (true)
	{

		/*if (controller[ControllerDigital::right].changedToPressed())
		{
			holdTray = !holdTray;
		}
		if (holdTray){
			trayLift.moveVelocity(200);
			pros::delay(0.3);
			trayLift.moveVelocity(-1);
			//backLiftPID(400);
		}
		else {
			trayLift.moveVelocity(slowTraySpeed * controller.getDigital(ControllerDigital::R1) +
							  fastTraySpeed * controller.getDigital(ControllerDigital::left) - fastTraySpeed * controller.getDigital(ControllerDigital::R2));
		}*/

		/*if (controller[ControllerDigital::A].changedToPressed())
		{
			holdLift = !holdLift;
		}

		if (holdLift)
		{
			//armLiftPID(400, 800);
			armLift.moveVelocity(1);
		}
		else {
			armLift.controllerSet(controller.getDigital(ControllerDigital::X) - controller.getDigital(ControllerDigital::B));
		}*/
		//std::cout << intakeLS.get_value() << " " << indexerLS.get_value() << " " << hoodLS.get_value() << " " << intakeBall << " " << indexerBall << " " << hoodBall << std::endl;
		chassis.arcade(controller.getAnalog(ControllerAnalog::leftY), controller.getAnalog(ControllerAnalog::rightX));

		//std::cout << "\nRoller Temperature:" << rollerOne.get_temperature();
	
		
		//std::cout <<buttonCount << " -- " << intakeSpeed <<std::endl;
		/*if (controller.getDigital(ControllerDigital::left)) {
			
			buttonCount++;
			if(buttonCount%2 == 1){
				intakeSpeed = 100;
			}
			else {
				intakeSpeed = 200;
			}
			pros::delay(300);
		}*/
 
		rollers.moveVelocity(200 * controller.getDigital(ControllerDigital::L1) - 200 * controller.getDigital(ControllerDigital::Y) - 50 * controller.getDigital(ControllerDigital::L2));

		pros::delay(20);
	 }
}

//void
/*void positionTracking(double x, double y){
	int rect = 0; 
	int length = 0;
	int chaswidth = 0; 
}*/

//Autonomous
void collectCubes()
{
	//Flip out
	rollers.moveVelocity(-200);
	pros::delay(400);
	rollers.moveVelocity(0);
	/*movePID(15, 15, fastMoveKP, 300);
	armLift.moveVelocity(200);
	pros::delay(1000);
	armLift.moveVelocity(0);*/
	trayLift.moveVelocity(-200);
	pros::delay(650);
	trayLift.moveVelocity(0);
	pros::delay(500);

	//Pick up the cubes
	rollers.moveVelocity(200);
	movePID(25, 25, slowMoveKP, 1500);
	pros::delay(2000);
	movePID(28, 28, slowMoveKP, 1500);

	//Move back to wall align
	//movePID(-43, -43, slowMoveKP, 2000);
	pros::delay(500);
	//rollers.moveVelocity(0);
}

void stackCubes()
{
	rollers.moveVelocity(0);
	//Get bottom cube in position to stack
	armLift.moveVelocity(-200);
	pros::delay(200);
	armLift.moveVelocity(0);
	rollers.moveVelocity(-100);
	pros::delay(400);
	rollers.moveVelocity(0);

	//Straighten up the tray and align bottom
	std::cout << "\nPot Value:" << trayPot.get_value();
	trayLiftPID(1300);
	std::cout << "\nPot Value:" << trayPot.get_value();
	trayLift.moveVelocity(0);
	movePID(8, 8, slowMoveKP, 300);

	// //Outtake the cubes and move backwards
	// rollers.moveVelocity(-70);
	// armLift.moveVelocity(-200);
	// pros::delay(200);
	// movePID(-15, -15, slowMoveKP, 1500);
	// rollers.moveVelocity(0);

}

void red()
{/*
	rollers.moveVelocity(-200);
	pros::delay(300);
	rollers.moveVelocity(200);
	//Flip out
	rollers.moveVelocity(-200);
	pros::delay(400);
	rollers.moveVelocity(0);

	trayLift.moveVelocity(-200);
	pros::delay(650);
	trayLift.moveVelocity(0);
	pros::delay(500);

	//Pick up the cubes
	rollers.moveVelocity(200);
	movePID(25, 25, slowMoveKP, 1500);
	pros::delay(2000);
	movePID(30, 30, slowMoveKP, 1500);

	//Move back to wall align
	//movePID(-43, -43, slowMoveKP, 2000);
	pros::delay(700);
	
	//Move forward, turn, and into goal
	//fastMovePID(10, 10, 900);
	movePID(-49, -49, slowMoveKP, 1000);
	rollers.moveVelocity(0);
	//pros::delay(2000);
	pros::delay(200); 
	movePID(27 , -27, slowMoveKP, 800);
	pros::delay(500);
	movePID(27, 27, slowMoveKP, 1000);
	pros::delay(500);
	//movePID(25, 25, slowMoveKP, 1000);

	rollers.moveVelocity(0);
	//Get bottom cube in position to stack
	armLift.moveVelocity(-200);
	pros::delay(200);
	armLift.moveVelocity(0);
	rollers.moveVelocity(-100);
	pros::delay(400);
	rollers.moveVelocity(0);*/

	//Straighten up the tray and align bottom
	std::cout << "\nPot Value:" << trayPot.get_value();
	//trayLift.moveVelocity(200);
	//pros::delay(2000);
	// holdTray=true;
	// trayPosition = 950;
	// pros::delay(4000);
	// holdTray=false;
	// //pros::delay(100000);
	backLiftPID(900);
	//holdTray2=false;
	movePID(-20,-20, slowMoveKP, 1000);
}

void redBig()
{
	rollers.moveVelocity(-200);
	pros::delay(300);
	rollers.moveVelocity(200);

	movePID(19.5, 19.5, slowMoveKP, 1000);
	pros::delay(100);
	movePID(-15.5, 15.5, slowMoveKP, 800);
	pros::delay(100);
	movePID(18, 18, slowMoveKP, 1000);
	pros::delay(100);
	movePID(-6.5, 6.5, slowMoveKP, 800);
	pros::delay(100);
	movePID(23.5, 23.5, slowMoveKP, 1000);

	stackCubes();
}

void push()
{
	movePID(30, 30, fastMoveKP, 1500);
	movePID(-30, -30, fastMoveKP, 1500);
}

void blue()
{
	collectCubes();
	rollers.moveVelocity(0);
	//Move forward, turn, and into goal
	//fastMovePID(10, 10, 900);
	movePID(-43, -43, slowMoveKP, 800);
	pros::delay(400);
	movePID(-21, 21, slowMoveKP, 800);
	pros::delay(400);
	movePID(14, 14, slowMoveKP, 1000);
	pros::delay(400);
	//movePID(25, 25, slowMoveKP, 1000);

	rollers.moveVelocity(0);
	//Get bottom cube in position to stack
	armLift.moveVelocity(-200);
	pros::delay(200);
	armLift.moveVelocity(0);
	rollers.moveVelocity(-100);
	pros::delay(400);
	rollers.moveVelocity(0);

	//Straighten up the tray and align bottom
	backLiftPID(930);
	trayLift.moveVelocity(0);
	movePID(5, 5, slowMoveKP, 300);

	//Outtake the cubes and move backwards
	rollers.moveVelocity(-70);
	armLift.moveVelocity(-200);
	pros::delay(200);
	movePID(-15, -15, slowMoveKP, 1500);
	rollers.moveVelocity(0);
}

void autonomous()
{
	switch (lcdCounter)
	{
	case 0:
		break;
	case 1:
		red();
		break;
	case 2:
		push();
		break;
	case 3:
		blue();
		break;
	case 4:
		redBig();
		break;
	}
}

bool selected = true; //TODO: false

void left_button()
{
	if (!selected)
	{
		lcdCounter--;
		if (lcdCounter < 0)
		{
			lcdCounter = 0;
		}
	}
}
void center_button()
{
	if (!selected)
	{
		selected = true;
	}
}
void right_button()
{
	if (!selected)
	{
		lcdCounter++;
		if (lcdCounter > 4)
		{
			lcdCounter = 4;
		}
	}
}
std::string convert(int arg)
{
	switch (arg)
	{
	case 0:
		return "No Auton";
	case 1:
		return "Red";
	case 2:
		return "Push";
	case 3:
		return "Blue";
	case 4:
		return "Red Big";
	default:
		return "No Auton";
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */

void initialize()
{
	pros::lcd::initialize();
	pros::lcd::register_btn0_cb(left_button);
	pros::lcd::register_btn1_cb(center_button);
	pros::lcd::register_btn2_cb(right_button);
	//pros::lcd::print(0, "Test Temperature", pros::trayLift::get_temperature());
	
	//intakeLS.calibrate();
	//rollers.calibrate();
	//indexerLS.calibrate();
	//hoodLS.calibrate();

	while (!selected)
	{
		pros::lcd::set_text(0, convert(lcdCounter));
		pros::delay(20);
	}

	pros::lcd::set_text(0, convert(lcdCounter) + " (SELECTED)");

	//pros::Task trayTaskHandle(trayTask);
	//pros::Task armTaskHandle(armTask);
	//pros::Task trayLiftTaskHandle(trayLiftTask);
}

//void disabled() {}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */

//void competitionitialize() {}