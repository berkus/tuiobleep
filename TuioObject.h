/*
	TUIO C++ backend - part of the reacTIVision project
	http://www.iua.upf.es/mtg/reacTable

	Copyright (c) 2006 Martin Kaltenbrunner <mkalten@iua.upf.es>

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files
	(the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software,
	and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	Any person wishing to distribute modifications to the Software is
	requested to send the modifications to the original developer so that
	they can be incorporated into the canonical version.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
	ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef INCLUDED_TUIOOBJECT_H
#define INCLUDED_TUIOOBJECT_H

#include <math.h>

class TuioObject { 
	
	public:
		unsigned int session_id, fiducial_id;
		float xpos, ypos, angle;

		TuioObject(unsigned int s_id, unsigned int f_id) { 
			session_id = s_id;
			fiducial_id = f_id;

			xpos = 0.0f;
			ypos = 0.0f;
			angle = 0.0f;
		}

		void update(float x, float y, float a) {
			xpos = round(640*x);
			ypos = round(640*y);
			angle = a/M_PI*180;
		}
};

#endif /* INCLUDED_TUIOCLIENT_H */
