/***************************************************************************
 *   Copyright (C) 2006 by Martin Domig                                    *
 *   martin@domig.net                                                      *
 *                                                                         *
 *   For license information see LICENSE.xivap in the root folder of the   *
 *   source code.                                                          *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 ***************************************************************************/

#include "helpers.h"

#include <stdio.h>
#include <math.h>

string ftoa(double d)
{
	char buf[512];
	sprintf(buf, "%.5f", d);
	string result = buf;
	return result;
}

string stripchar(const string& str, char c)
{
	string result = str;
	int p = 0;
	while((p = pos(c, result)) > 0)
		del(result, p, 1);
	return result;
}

string stripcolons(const string& str)
{
	string result = str;
	int p = 0;
	while((p = pos(':', result)) > 0) result[p] = ' ';
	return result;
}

double deg2dist(double lat1, double lon1, double lat2, double lon2)
{
	// convert to radians
	lat1 = lat1 / 180.0 * PI;
	lat2 = lat2 / 180.0 * PI;
	lon1 = -lon1 / 180.0 * PI;
	lon2 = -lon2 / 180.0 * PI;

	// precompute...
	double sin1 = (sin((lat1-lat2)/2.0));
	double sin2 = (sin((lon1-lon2)/2.0));

	// return result in NM
	return 2 * asin(sqrt(sin1*sin1 + cos(lat1)*cos(lat2)*(sin2*sin2))) * 180 * 60 / PI;
}

double deg2crs(double lat1, double lon1, double lat2, double lon2)
{
	// convert to radians
	lat1 = lat1 / 180.0 * PI;
	lat2 = lat2 / 180.0 * PI;
	lon1 = -lon1 / 180.0 * PI;
	lon2 = -lon2 / 180.0 * PI;

	// precompute...
	double sin1 = (sin((lat1-lat2)/2.0));
	double sin2 = (sin((lon1-lon2)/2.0));

	double d = 2 * asin(sqrt(sin1*sin1 + cos(lat1)*cos(lat2)*(sin2*sin2)));

	double res;

	if(sin(lon2 - lon1) < 0)       
		res = acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));
	else
		res = 2 * PI - acos((sin(lat2) - sin(lat1) * cos(d)) / (sin(d) * cos(lat1)));

	// convert to degrees
	res = res * 180.0 / PI;
	while(res < 0) res += 360.0;
	while(res > 360) res -= 360.0;

	return res;
}


#define ISBLANK(c) (c == ' ' || c == '\r' || c == '\n' || c == '\t')

string trim(string line)
{
	while(length(line) > 0 && ISBLANK(line[0])) // strip left blanks
		del(line, 0, 1);
	
	while(length(line) > 0 && ISBLANK(line[length(line)-1])) // strip right blanks
		del(line, length(line)-1, 1);

	return line;
}

string strupcase(string line)
{
	for(int i = 0; i < length(line); ++i)
		line[i] = pt::upcase(line[i]);

	return line;
}

string getXplaneHomeDir(void)
{
	static bool firsttime = true;
	if (firsttime) {
#if APL
		if (XPLMIsFeatureEnabled("XPLM_USE_NATIVE_PATHS") == 0)
		{
			XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1); //for native path MacOs
		}
#endif
		firsttime = false;
	}
	char app[512];
	XPLMGetSystemPath(app);
	string result=("");
	
	
	if(app != NULL) {
		string path = string(app);
		int p;



		p = rpos(DIR_CHAR, path);
		result=copy(path,0,p) +DIR_CHAR;
		return  result;
	}
	return string("");
}

string getXivapRessourcesDir()
{

	string path=getXplaneHomeDir( );
	
	//const char* app = GetApplicationPath();
	static bool firsttime = true;

	path += ((pos(string(".app/Contents"), path) > 0) ? string("../../../") : string("")) 
	        + string("Resources") + DIR_CHAR + "plugins" + DIR_CHAR
			+ string(RESOURCES_DIR) + DIR_CHAR;

	if(firsttime) {
		XPLMDebugString(string("My resources directory is: ") + path + "\n");
		firsttime = false;
	}

	return path;
}



string float2coords(const float& ord)
{
    int deg = static_cast<int>(ord);
    float secs = (ord - deg) * 60;
    if(secs < 0) secs *= -1;
    string result = itostring(deg) + ".";
	
	float ord2 = secs;
	deg = static_cast<int>(ord2);
    secs = (ord2 - deg) * 60;
    if(secs < 0) secs *= -1;
    result += itostring(deg) + "." + itostring(static_cast<int>(secs));
    return result;
}

bool is25kHzFreq(string freq)
{
    if(length(freq) != 7 || freq[3] != '.') {
        return false; // invalid frequency
    }
    
    string lastTwo = copy(freq,5,2); 
    if(lastTwo =="00" || lastTwo =="25" || lastTwo =="50" || lastTwo =="75") {
        return true;
    } else {
       return false;
    }
}
