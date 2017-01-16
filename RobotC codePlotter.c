#include "NXT_FileIO.c"

void setPen(bool down);
void zeroPen();
void zeroHead();
void zeroPaper();
void ejectPen();
void ejectPaper();
int selectSpeed();
void changeColour(int colour);
void moveTo(int x, int y, int movementPower);
//Deltas for pen
const int DELTA_UP = 10;
const int DELTA_DOWN = -7;

task main()
{
	//Initilize sensors
    SensorType[S1] = sensorTouch;
    SensorType[S2] = sensorColorNxtFULL;
	SensorType[S3]= sensorTouch;
	//Varibles
    int movementPower=50;
 	string colours[5]={"Black","Blue","Green", "Yellow","Red"};
 	

	//Found these in the documentation
    bFloatDuringInactiveMotorPWM = false;//Locks motor when not moving
    bMotorReflected[motorB] = true;//reflect Motor B


	//Initial setup
    zeroHead();
    displayString(0, "Insert Paper");
    zeroPaper();
    eraseDisplay();
	
	//Read file
    TFileHandle fin;
    bool fileOkay = openReadPC(fin, "tron (2).txt");

    if (!fileOkay)
    {
        displayString(0, "Failed To Open File");
        while (nNxtButtonPressed==-1);
    }
    else
    {
    	//States 1 pen down 0 pen up -1 end of commands
        int state = 0, x = 0, y = 0, colour = 0;
        //While there are still command sets
        while (readIntPC(fin, colour))
        {
  
			changeColour(colour);
            movementPower=selectSpeed();

			//Draw command set
            state=0;
            while (state != -1)
            {
                readIntPC(fin, state);
                if (state != -1)
                {
                    setPen(state);
                    readIntPC(fin, x);
                    readIntPC(fin, y);
                    //Displays colour in use position to move to
                    displayClearTextLine(0);
                    displayClearTextLine(1);
                    displayString(0, "Colour: %s", colours[colour-1]);
                    displayString(1, "Go To: %d %d", x, y);
                    moveTo(x, y,movementPower);
                }
            }
            setPen(0); //Take pen off paper at end of line
        }
        //End of file eject drawing and pen
        ejectPen();
        ejectPaper();
    }
}

void changeColour(int colour)
{
	      	//Set Pen to colour
           int readColour = 0;
           displayClearTextLine(0);
           displayString(0, "Insert %s", colours[colour-1]);
            do
            {
                ejectPen();
                zeroPen();
                readColour = SensorValue[S2];
            } while (readColour != colour);
}

void ejectPen()
{
    motor[motorA] = 100;
    while (nMotorEncoder[motorA] < 300);
    motor[motorA] = 0;
}

void ejectPaper()
{
	motor[motorC] = 100;
    while (nMotorEncoder[motorC] < 1000);
    motor[motorC] = 0;
}

void setPen(bool down)
{
	//Does nothing is already in desired state
    if (!down)
    {
        motor[motorA] = 100;
        while (nMotorEncoder[motorA] < DELTA_UP);
    }
    else
    {
        motor[motorA] = -100;
        while (nMotorEncoder[motorA] > DELTA_DOWN);
    }
    motor[motorA] = 0;
}


void zeroHead()
{
	//Moves to touch sensor
    motor[motorB] = 25;
    while (!SensorValue[S1]);

    motor[motorB] = 0;
    nMotorEncoder[motorB] = 0;
    motor[motorB] = -10;
	
	//Moves back a bit before zeroing
    while (nMotorEncoder[motorB] > -30);
    motor[motorB] = 0;
    wait1Msec(100);
    nMotorEncoder[motorB] = 0;
}

void zeroPen()
{
	//Button 3 signifies end of zeroing
    while (nNxtButtonPressed != 3)
    {
        if (nNxtButtonPressed == 1)
            motor[motorA] = -10;
        else if (nNxtButtonPressed == 2)
            motor[motorA] = 10;
        else
            motor[motorA] = 0;
    }
    nMotorEncoder[motorA] = 0;
    while (nNxtButtonPressed == 3);
}

void zeroPaper()
{
	//Button 3 signifies end of zeroing
    while (nNxtButtonPressed != 3)
    {
        if (nNxtButtonPressed == 1)
            motor[motorC] = -25;
        else if (nNxtButtonPressed == 2)
            motor[motorC] = 25;
        else
            motor[motorC] = 0;
    }
    wait1Msec(10)
    //Moves the paper slightly to adjust for play in gears
    nMotorEncoder[motorC] = 0;
    motor[motorC] = -10;
    while (nMotorEncoder[motorC] > -30);
    motor[motorC] = 0;
    while (nNxtButtonPressed == 3);
    nMotorEncoder[motorC] = 0;
}

// moves header to specified point on the paper
void moveTo(int x, int y, int movementPower) 
{

    //Calculate motor powers using realated triangles
    int dx = x - nMotorEncoder[motorB];
    int dy = y - nMotorEncoder[motorC];
    double dt = sqrt(1.0*dx*dx+1.0*dy*dy);
    int powerX =0;
    int powerY =0;
    if(dx!=0)
     powerX = (round(dx*(movementPower/dt);
   if(dy!=0)
   	 powerY = (round(dy*(movementPower/dt));

	//Determine if moving in positive or negitive direction
    bool xpos = dx > 0, ypos = dy > 0;
    motor[motorB]=powerX;
    motor[motorC]=powerY;

    //Once one of the dimentions have reached
    while ((xpos ? nMotorEncoder[motorB] < x : nMotorEncoder[motorB] > x)
        && (ypos ? nMotorEncoder[motorC] < y : nMotorEncoder[motorC] > y))
      {
       if(SensorValue[S3]==1)
       stopAllTasks();
    	}

    if ((xpos ? nMotorEncoder[motorB] > x : nMotorEncoder[motorB] < x)
        && (ypos ? nMotorEncoder[motorC] > y : nMotorEncoder[motorC] < y))
    {
        motor[motorB] = 0;
        motor[motorC] = 0;
    }
    //X is reached first
    if (xpos ? nMotorEncoder[motorB] > x : nMotorEncoder[motorB] < x)
    {
        motor[motorB] = 0;
        while (ypos ? nMotorEncoder[motorC] < y : nMotorEncoder[motorC] > y)
       	 {
       if(SensorValue[S3]==1)
       stopAllTasks();//found in help documentation
    			}
            motor[motorC] = 0;
    }
    //If Y reached first
    if (ypos ? nMotorEncoder[motorC] > y : nMotorEncoder[motorC] < y)
    {
        motor[motorC] = 0;
        while (ypos ? nMotorEncoder[motorC] < y : nMotorEncoder[motorC] > y)
        	      {
       if(SensorValue[S3]==1)
       stopAllTasks();
    	}
        motor[motorB] = 0;
    }

	//Just in case
    motor[motorB] = 0;
    motor[motorC] = 0;

    //Displays position move to by encoders
	displayClearTextLine(2);
    displayString(2, "From: %d %d",nMotorEncoder[motorB],nMotorEncoder[motorC]);

    wait1Msec(10);
}

int selectSpeed()
{
	int movementPower=50;
	while (nNxtButtonPressed!=3)
	{
		if (nNxtButtonPressed==1)
		{
			while (nNxtButtonPressed==1);
			if (movementPower<100)
				movementPower+=10;
		}
		else if(nNxtButtonPressed==2)
			{
			while (nNxtButtonPressed==2);
			if (movementPower>10)
				movementPower-=10;
		}

       displayString(0, "Select speed: %d", movementPower);
     }
		return movementPower;
  }
