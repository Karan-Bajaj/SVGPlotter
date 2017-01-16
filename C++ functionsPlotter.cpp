#include <queue>
#include <list>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>

using namespace std;

string parsePath(istream & fin, double scaleFactor,
	double & xInit, double & yInit);
string parseCubic(double cx1, double  cy1, double  cx2, double  cy2,
	double  xi, double  yi, double  xf, double  yf, double scaleFactor);
string parseQuad(double cx, double  cy, double  xi, double  yi,
	double  xf, double yf, double scaleFactor);
string parseMove(double  xf, double yf, double scaleFactor);
string parseLine(double  xf, double yf, double scaleFactor);
string parseLine(istream & fin, double scaleFactor,
	double & xInit, double & yInit);
string parseCircle(istream & fin, double scaleFactor,
	double & xInit, double & yInit);
string parseRectangle(istream & fin, double scaleFactor,
	double & xInit, double & yInit);

//Karan Bajaj
int main()
{

	//Measured values from the plotter
	const double PLOTTER_HEIGHT = 500;
	const double PLOTTER_WIDTH = 460;

	ifstream fin("20856.svg");//modified svg file
	if (!fin)
	{
		cout << "Failed to open file";
		return -1;
	}

	string temp = "", commandSet = "", command = "1";
	double width = 0, height = 0, scaleFactor = 0;
	bool parseFile = 0;
	double x = 0, y = 0;
	int colour = 0;
	string colours[6] = { "" };//Command sets for each colour

							   //Finds start of svg file
	while (fin >> temp && !parseFile)
	{
		if (temp == "svg")
		{
			fin >> width >> height;
			scaleFactor = min((PLOTTER_HEIGHT / height),
				(PLOTTER_WIDTH / width));
			parseFile = 1;
		}
	}

	//Parses svg command sets
	do //do loop because first command is read in when finding the start
	{
		fin >> colour;
		if (temp == "path")
		{
			commandSet = parsePath(fin, scaleFactor, x, y);
		}
		else if (temp == "rect")
		{
			commandSet = parseRectangle(fin, scaleFactor, x, y);
		}
		else if (temp == "line")
		{
			commandSet = parseLine(fin, scaleFactor, x, y);
		}
		colours[colour - 1] += commandSet;
	} while (fin >> temp&&parseFile&&temp != "/svg>");

	//outputs commands to file
	ofstream fout("image.txt");
	for (int i = 0; i<6; i++)
	{
		if (colours[i] != "")//if not empty
			fout << i + 1 << " " << colours[i] << " -1" << endl;
	}
	fin.close();
	fout.close();
	return 0;
}

//Eric Chee
string parsePath(istream & fin, double scaleFactor, double & xInit,
	double & yInit)
{
	string junk, temp;
	double cx1, cy1, cx2, cy2, xChange, yChange, xStart = xInit, yStart = yInit;
	stringstream commandSet;
	char command = ' ', prevCommand = ' ';

	//Lower case is realitive to previous
	//Upper case is absoute to 0, 0

	//Parse sub commands
	while (command != '>')//'>' is the end of path sentry
	{
		fin >> command;

		//Cubic curves
		if (command == 'c' || command == 'C')
		{
			fin >> cx1 >> cy1 >> cx2 >> cy2 >> xChange >> yChange;
			if (command == 'C')
			{
				xChange -= xStart;
				yChange -= yStart;
			}
			commandSet << parseCubic(cx1, cy1, cx2, cy2, xStart, yStart,
				xChange, yChange, scaleFactor);
		}
		//Shorthand cubic curves
		else if (command == 's' || command == 'S')
		{
			if (prevCommand == 'c' || prevCommand == 's' ||
				prevCommand == 'C' || prevCommand == 'S')
			{
				cx1 = xStart + (xStart - cx2);
				cy1 = xStart + (xStart - cy2);
			}
			else
			{
				cx1 = xStart;
				cy1 = yStart;
			}
			fin >> cx2 >> cy2 >> xChange >> yChange;
			if (command == 'S')
			{
				xChange -= xStart;
				yChange -= yStart;

			}
			commandSet << parseCubic(cx1, cy1, cx2, cy2, xStart, yStart,
				xChange, yChange, scaleFactor);
		}

		//Quadratic Curves
		else if (command == 'q' || command == 'Q')
		{
			fin >> cx1 >> cy1 >> xChange >> yChange;
			if (command == 'Q')
			{
				xChange -= xStart;
				yChange -= yStart;
			}
			commandSet << parseQuad(cx1, cy1, xStart, yStart,
				xChange, yChange, scaleFactor);

		}
		//Shorthand quadratic curves
		else if (command == 't' || command == 'T')
		{
			if (prevCommand == 't' || prevCommand == 'q' ||
				prevCommand == 'T' || prevCommand == 'Q')
			{
				cx1 = xStart + (xStart - cx1);
				cy1 = yStart + (yStart - cy1);
			}
			else
			{
				cx1 = xStart;
				cy1 = yStart;
			}
			fin >> xChange >> yChange;
			commandSet << parseQuad(cx1, cy1, xStart, yStart,
				xChange, yChange, scaleFactor);
			if (command == 'T')
			{
				xChange -= xStart;
				yChange -= yStart;
			}

		}

		//(Straight) Lines
		else if (command == 'l')
		{
			fin >> xChange >> yChange;
			commandSet << parseLine(xStart + xChange,
				yStart + yChange, scaleFactor);
		}
		else if (command == 'L')
		{
			fin >> xChange >> yChange;
			commandSet << parseLine(xChange, yChange, scaleFactor);
		}

		//Move to
		else if (command == 'm')
		{
			fin >> xChange >> yChange;
			commandSet << parseMove(xStart + xChange,
				yStart + yChange, scaleFactor);
		}
		else if (command == 'M')
		{
			fin >> xChange >> yChange;
			commandSet << parseMove(xChange, yChange, scaleFactor);

		}

		//Return to start
		else if (command == 'z' || command == 'Z')
		{
			xChange = xStart - xInit;
			yChange = yStart - yInit;
			commandSet << parseLine(xChange, yChange, scaleFactor);
		}

		prevCommand = command;
		xStart += xChange;
		yStart += xChange;
	}

	//sets the current position of the head 
	xInit = xStart;
	yInit = yStart;

	return commandSet.str();
}


//Note all coordinates are scaled at the point of cobversion to plotter points
//to in avoid confusion of what scale point are in other methods 

//Karan
string parseRectangle(istream & fin, double scaleFactor,
	double & xInit, double & yInit)
{
	double xi, yi, width, height;

	fin >> xi >> yi >> width >> height;
	xInit = round(xi);
	yInit = round(yi);
	xi *= scaleFactor;
	yi *= scaleFactor;
	width *= scaleFactor;
	height *= scaleFactor;

	stringstream commandStream;

	commandStream << " 0 " << round(xi) << " " << round(yi) << " "
		<< "1 " << round(xi + width) << " " << round(yi) << " "
		<< "1 " << round(xi + width) << " " << round(yi + height) << " "
		<< "1 " << round(xi) << " " << round(yi + height) << " "
		<< "1 " << round(xi) << " " << round(yi) << " ";
	return commandStream.str();
}

//Line command Karan
string parseLine(istream & fin, double scaleFactor,
	double & xInit, double & yInit)
{
	double xi, yi, xf, yf;

	fin >> xi >> yi >> xf >> yf;
	xInit = round(xf);
	yInit = round(yf);
	xi *= scaleFactor;
	yi *= scaleFactor;
	xf *= scaleFactor;
	yf *= scaleFactor;

	stringstream commandStream;

	commandStream << " 0 " << round(xi) << " " << round(yi) << " "
		<< "1 " << round(xf) << " " << round(yf) << " ";

	return commandStream.str();
}

//Line subcommand of Path Karan or Mulan?
string parseLine(double  xf, double yf, double scaleFactor)
{
	xf *= scaleFactor;
	yf *= scaleFactor;
	stringstream commandStream;

	commandStream << "1 " << round(xf) << " " << round(yf) << " ";
	return commandStream.str();
}

//Mulan
string parseMove(double  xf, double yf, double scaleFactor)
{
	xf *= scaleFactor;
	yf *= scaleFactor;
	stringstream commandStream;

	commandStream << "0 " << round(xf) << " " << round(yf) << " ";

	return commandStream.str();
}

//Quadratic curve Ken Lee
string parseQuad(double cx, double cy, double xi, double yi,
	double xf, double yf, double scaleFactor)
{

	cx = (xi + cx)*scaleFactor;
	cy = (yi + cy)*scaleFactor;
	xf = (xi + xf)*scaleFactor;
	yf = (yi + yf)*scaleFactor;
	xi *= scaleFactor;
	yi *= scaleFactor;

	double xLast = xi,
		xCurrent = 0.0,
		yLast = yi,
		yCurrent = 0.0;

	double a1x = xi - 2 * cx + xf,
		a2x = 2 * cx - 2 * xi,
		a1y = yi - 2 * cy + yf,
		a2y = 2 * cy - 2 * yi;

	stringstream cood;

	double t = 0;
	for (; t<1.00; t += 0.01)
	{
		do
		{
			t += 0.01;
			xCurrent = a1x*t*t + a2x*t + xi;
			yCurrent = a1y*t*t + a2y*t + yi;
		} while (abs(xCurrent - xLast)<2.0 || abs(yCurrent - yLast)<2.0);

		cood << " 1 " << round(xCurrent) << " " << round(yCurrent);
		yLast = yCurrent;
		xLast = xCurrent;
	}

	if (xCurrent != xf || yLast != yf)
	{
		cood << " 1 " << round(xf) << " " << round(yf);
	}

	return cood.str();
}

//Cubic curve Karan
string parseCubic(double cx1, double cy1, double cx2, double cy2,
	double xi, double yi, double xf, double yf,
	double scaleFactor)
{
	cx1 = (xi + cx1)*scaleFactor;
	cy1 = (xi + cy1)*scaleFactor;
	cx2 = (xi + cx2)*scaleFactor;
	cy2 = (yi + cy2)*scaleFactor;
	xf = (xi + xf)*scaleFactor;
	yf = (yi + yf)*scaleFactor;
	xi *= scaleFactor;
	yi *= scaleFactor;

	double xCurrent = 0,
		xLast = xi,
		yLast = yi,
		yCurrent = 0;

	double a3x = 3 * (cx1 - xi),
		a2x = 3 * (cx2 - cx1) - a3x,
		a1x = xf - xi - a3x - a2x,
		a3y = 3 * (cy1 - yi),
		a2y = 3 * (cy2 - cy1) - a3y,
		a1y = yf - yi - a3y - a2y,
		t = 0;

	stringstream cood;

	for (; t<1.0; t += 0.01)
	{
		do
		{
			t += 0.01;
			xCurrent = a1x*t*t*t + a2x*t*t + a3x*t + xi;
			yCurrent = a1y*t*t*t + a2y*t*t + a3y*t + yi;
		} while (abs(xCurrent - xLast)<2.0 || abs(yCurrent - yLast)<2.0);

		cood << " 1 " << round(xCurrent) << " " << round(yCurrent);
		yLast = yCurrent;
		xLast = xCurrent;
	}

	if (xCurrent != xf || yLast != yf)
	{
		cood << " 1 " << round(xf) << " " << round(yf);
	}
	return cood.str();
}
