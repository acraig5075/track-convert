#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "tinyxml2.h"

using namespace std;
using namespace tinyxml2;

namespace trkconv
{
class Converter;
class Reader;
class Writer;
class GpxReader;
class TcxReader;
class KmlWriter;
class ReaderFactory;
class WriterFactory;

struct Point;
struct Params;
struct Utils;

typedef vector<Point> Points;
typedef vector<Points> Tracks;

enum FileType { UNKNOWN = 0, CSV, GPX, TCX, KML };

/*******************************************************************/

struct Point
{
	double oLat = 0.0;
	double oLon = 0.0;
	double oEle = 0.0;
};

/*******************************************************************/

struct Params
{
	Tracks oTracks;
	string oInFilename;
	string oOutFilename;
	FileType oInType;
	FileType oOutType;
	int oError = 0;
};

/*******************************************************************/

class Reader
{
	virtual bool NextTrack() = 0;
	virtual bool NextPoint(Point &) = 0;
protected:
	Params &oParams;
	XMLDocument oDoc;
	const XMLElement *oRoot = nullptr;
	bool AddTrack();
public:
	Reader(Params &);
	virtual ~Reader() {};
	bool Read();
};

/*******************************************************************/

class Writer
{
protected:
	Params &oParams;
public:
	Writer(Params &);
	virtual ~Writer() {};
	bool Write();
	virtual void WritePoint(const Point &) = 0;
	virtual void PreWrite() {};
	virtual void PostWrite() {};
	virtual void PreWritePoints() {};
	virtual void PostWritePoints() {};
};

/*******************************************************************/

class Converter
{
	Params oParams;
public:
	Converter();
	void SetInFilename(string);
	void SetOutFilename(string);
	void SetOutFileType(FileType);
	bool Convert();
	string GetErrorMsg() const;
};

/*******************************************************************/

class GpxReader : public Reader
{
	const XMLElement *oTrk = nullptr;
	const XMLElement *oTrkPt = nullptr;
	const XMLElement *oTrkSeg = nullptr;
public:
	GpxReader(Params &);
	bool NextTrack();
	bool NextPoint(Point &pnt);
};

/*******************************************************************/

class TcxReader : public Reader
{
	const XMLElement *oActivities = nullptr;
	const XMLElement *oActivity = nullptr;
	const XMLElement *oLap = nullptr;
	const XMLElement *oTrk = nullptr;
	const XMLElement *oTrkPt = nullptr;
public:
	TcxReader(Params &);
	bool NextTrack();
	bool NextPoint(Point &pnt);
};

/*******************************************************************/

class CsvWriter : public Writer
{
public:
	CsvWriter(Params &);
	void WritePoint(const Point &);
};

/*******************************************************************/

class KmlWriter : public Writer
{
	XMLDocument oDoc;
	XMLElement *oRoot = nullptr;
	XMLElement *oCoords = nullptr;
	ostringstream oStream;
public:
	KmlWriter(Params &);
	void WritePoint(const Point &);
	void PreWrite();
	void PostWrite();
	void PreWritePoints();
	void PostWritePoints();
};

/*******************************************************************/

class ReaderFactory
{
public:
	static Reader *NewReader(FileType, Params &);
};

/*******************************************************************/

class WriterFactory
{
public:
	static Writer *NewWriter(FileType, Params &);
};

/*******************************************************************/

struct Utils
{
	static bool FileExists(string);
	static FileType GetInFileType(string);
};

} // trkconv namespace