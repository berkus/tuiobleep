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

#include "TuioClient.h"


#ifndef WIN32
static void* ThreadFunc( void* obj )
#else
static DWORD WINAPI ThreadFunc( LPVOID obj )
#endif
{
	static_cast<TuioClient*>(obj)->socket->Run();
	return 0;
};

TuioClient::TuioClient() {
	TuioClient(3333);
}

TuioClient::TuioClient(int port) {
	socket = new UdpListeningReceiveSocket(IpEndpointName( IpEndpointName::ANY_ADDRESS, port ), this );
	locked = false;
}

TuioClient::~TuioClient() {
	delete socket;
}

void TuioClient::ProcessBundle( const ReceivedBundle& b, const IpEndpointName& remoteEndpoint) {
	for( ReceivedBundle::const_iterator i = b.ElementsBegin(); i != b.ElementsEnd(); ++i ){
		if( i->IsBundle() )
			ProcessBundle( ReceivedBundle(*i), remoteEndpoint);
		else
			ProcessMessage( ReceivedMessage(*i), remoteEndpoint);
	}
}


void TuioClient::ProcessMessage( const ReceivedMessage& m, const IpEndpointName& remoteEndpoint) {
    	try {
		ReceivedMessageArgumentStream args = m.ArgumentStream();
		ReceivedMessage::const_iterator arg = m.ArgumentsBegin();

		if( strcmp( m.AddressPattern(), "/tuio/2Dobj" ) == 0 ){

			const char* cmd;
			args >> cmd;

			if( strcmp( cmd, "set" ) == 0 ){
				int32 s_id, f_id;
				float x, y, a, X, Y, A, m, r;

				args >> s_id >> f_id >> x >> y >> a >> X >> Y >> A >> m >> r >> EndMessage;

				std::list<id_pair>::iterator iter;
				for (iter=objectList.begin(); iter != objectList.end(); iter++)
					if(iter->s_id==(unsigned int)s_id) break;

				if (iter == objectList.end()) {
					id_pair add_pair;
					add_pair.s_id=(unsigned int)s_id;
					add_pair.f_id=(unsigned int)f_id;
					objectList.push_back(add_pair);
					listener->addTuioObj((unsigned long)s_id,(unsigned int)f_id);
				}

				listener->updateTuioObj((int)s_id,(int)f_id,x,y,a,X,Y,A,m,r);

			} else if( strcmp( cmd, "alive" ) == 0 ){

				int32 s_id;
				while(!args.Eos()) {
					args >> s_id;
					buffer.push_back((unsigned int)s_id);

					std::list<unsigned int>::iterator iter;
					iter = find(aliveList.begin(), aliveList.end(), (unsigned int)s_id);
					if (iter != aliveList.end()) aliveList.erase(iter);
				}
				args >> EndMessage;

				std::list<unsigned int>::iterator alive_iter;
				for (alive_iter=aliveList.begin(); alive_iter != aliveList.end(); alive_iter++) {
					std::list<id_pair>::iterator object_iter;
					for (object_iter=objectList.begin(); object_iter != objectList.end(); object_iter++) {
						if(object_iter->s_id==*alive_iter) {
							listener->removeTuioObj(object_iter->s_id,object_iter->f_id);
							objectList.erase(object_iter);
							break;
						}
					}

				}

				std::list<unsigned int> newList = aliveList;
				aliveList = buffer;

				// recycling the list
				buffer = newList;
				buffer.clear();
			} else if( strcmp( cmd, "fseq" ) == 0 ){

				int32 lastFrame = currentFrame;
				args >> currentFrame  >> EndMessage;
				if (currentFrame>lastFrame) listener->refresh();
			}
		}
	} catch( Exception& e ){
		std::cout << "error while parsing message: "<< m.AddressPattern() << ": " << e.what() << "\n";
	}
}

void TuioClient::ProcessPacket( const char *data, int size, const IpEndpointName& remoteEndpoint ) {
	if (listener==NULL) return;
	ReceivedPacket p( data, size );
	if(p.IsBundle()) ProcessBundle( ReceivedBundle(p), remoteEndpoint);
        else ProcessMessage( ReceivedMessage(p), remoteEndpoint);
}

void TuioClient::start(bool lk) {

	locked = lk;
	if (!locked) {
		#ifndef WIN32
		pthread_create(&thread , NULL, ThreadFunc, this);
		#else
		DWORD threadId;
		thread = CreateThread( 0, 0, ThreadFunc, this, 0, &threadId );
		#endif
	} else socket->Run();
}

void TuioClient::stop() {
	socket->Break();

	if (!locked) {
		#ifdef WIN32
		if( thread ) CloseHandle( thread );
		#endif
		thread = 0;
		locked = false;
	}
}

void TuioClient::addTuioListener(TuioListener *l) {
	listener = l;
}

void TuioClient::removeTuioListener() {
	listener = NULL;
}
