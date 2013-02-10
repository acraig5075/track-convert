#include "trkconv.h"
#include <iostream>
#include <iomanip>
#include <exception>
#include <map>

namespace trkconv
{

/*******************************************************************/

Reader::Reader(Params &params)
: oParams(params) 
{}

bool Reader::Read()
{ 
	cout << "Reading " << oParams.oInType << "...\n";

	oDoc.LoadFile(oParams.oInFilename.c_str());
	oRoot = oDoc.RootElement();
	if (oRoot == nullptr)
	{
		oParams.oError = 7;
		return false;
	}

	Point p;
	int i = 0;
	while (NextTrack())
	{
		Points &pnts = oParams.oTracks.at(i);
		while (NextPoint(p))
		{
			pnts.push_back(p);
		}
		++i;
	}
	return true;
}

bool Reader::AddTrack()
{
	Points p;
	oParams.oTracks.push_back(p);
	return true;
}

/*******************************************************************/

Writer::Writer(Params &params)
: oParams(params) 
{}

bool Writer::Write()
{
	cout << "Writing...\n";

	cout << oParams.oTracks.size() << " Tracks\n";
	if (oParams.oTracks.empty())
		return false;
	else
	{
		cout << oParams.oTracks.at(0).size() << " Points\n";

		PreWrite();

		for (auto track : oParams.oTracks)
		{
			PreWritePoints();

			for (Point p : track)
			{
				WritePoint(p);
			}

			PostWritePoints();
		}

		PostWrite();

		return true; 
	}
}

/*******************************************************************/

Converter::Converter()
{}

void Converter::SetInFilename(string filename)
{
	oParams.oInFilename.clear();
	oParams.oInType = UNKNOWN;

	if (Utils::FileExists(filename))
	{
		auto type = Utils::GetInFileType(filename);
		if (type != UNKNOWN)
		{
			oParams.oInFilename = filename;
			oParams.oInType = type;
		}
		else
			oParams.oError = 6;
	}
	else
		oParams.oError = 5;
};

void Converter::SetOutFilename(string filename)
{
	oParams.oOutFilename = filename;
};

bool Converter::Convert()
{
	if (UNKNOWN == oParams.oInType)
		oParams.oError = 1;
	else if (UNKNOWN == oParams.oOutType)
		oParams.oError = 2;
	else
	{
		auto oReader = ReaderFactory::NewReader(oParams.oInType, oParams);
		auto oWriter = WriterFactory::NewWriter(oParams.oOutType, oParams);

		if (nullptr == oReader)
			oParams.oError = 3;
		else if (nullptr == oWriter)
			oParams.oError = 4;
		else
		{
			try
			{
				if (oReader->Read())
					oWriter->Write();
			}
			catch (exception &)
			{
				cerr << "Exception caught\n";
			}
		}

		if (oReader)
			delete oReader;
		if (oWriter)
			delete oWriter;
	}

	return 0 == oParams.oError;
}

void Converter::SetOutFileType(FileType type)
{
	switch (type)
	{
		case CSV:
		case GPX:
		case TCX:
		case KML:
			oParams.oOutType = type;
			break;
		default:
			oParams.oOutType = UNKNOWN;
			break;
	}
}

string Converter::GetErrorMsg() const
{
	stringstream ss;
	ss << oParams.oError;
	return ss.str();
}

/*******************************************************************/

GpxReader::GpxReader(Params &params)
: Reader(params)
{}

bool GpxReader::NextTrack()
{
	oTrkPt = nullptr;

	if (!oRoot)
		throw exception();

	if (oTrkSeg)
	{
		oTrkSeg = oTrk->NextSiblingElement("trkseg");
		if (oTrkSeg)
			return AddTrack();
	}

	if (oTrk)
		oTrk = oTrk->NextSiblingElement("trk");
	else
		oTrk = oRoot->FirstChildElement("trk");

	if (oTrk)
		oTrkSeg = oTrk->FirstChildElement("trkseg");

	if (oTrkSeg)
		return AddTrack();

	return false;
}

bool GpxReader::NextPoint(Point &pnt)
{
	if (!oTrkSeg)
		throw exception();

	if (oTrkPt)
		oTrkPt = oTrkPt->NextSiblingElement("trkpt");
	else
		oTrkPt = oTrkSeg->FirstChildElement("trkpt");

	if (oTrkPt)
	{
		pnt.oLat = oTrkPt->DoubleAttribute("lat");
		pnt.oLon = oTrkPt->DoubleAttribute("lon");
		pnt.oEle = 0.0;
		auto ele = oTrkPt->FirstChildElement("ele");
		if (ele && ele->QueryDoubleText(&pnt.oEle))
			pnt.oEle = 0.0;
		return true;
	}

	return false;
}

/*******************************************************************/

TcxReader::TcxReader(Params &params)
: Reader(params)
{}

bool TcxReader::NextTrack()
{
	oTrkPt = nullptr;

	if (!oRoot)
		throw exception();

	if (!oActivities)
		oActivities = oRoot->FirstChildElement("Activities");
	if (!oActivity && oActivities)
		oActivity = oActivities->FirstChildElement("Activity");
	if (!oLap && oActivity)
		oLap = oActivity->FirstChildElement("Lap");
	if (!oActivities || !oActivity || !oLap)
		return false;

	if (oTrk)
	{
		oTrk = oTrk->NextSiblingElement("Track");
		if (oTrk)
			return AddTrack();
	}
	else
	{
		oTrk = oLap->FirstChildElement("Track");
		if (oTrk)
			return AddTrack();
		else
			return false;
	}

	oLap = oLap->NextSiblingElement("Lap");
	if (oLap)
	{
		oTrk = oLap->FirstChildElement("Track");
	}
	else
	{
		oActivity = oActivity->NextSiblingElement("Activity");
		if (oActivity)
		{
			oLap = oActivity->FirstChildElement("Lap");
			if (oLap)
			{
				oTrk = oLap->FirstChildElement("Track");
			}
		}
		else
		{
			oActivities = oActivities->NextSiblingElement("Activities");
			if (oActivities)
			{
				oActivity = oActivities->FirstChildElement("Activity");
				if (oActivity)
				{
					oLap = oActivity->FirstChildElement("Lap");
					if (oLap)
					{
						oTrk = oLap->FirstChildElement("Track");
					}
				}
			}
		}
	}

	if (oTrk)
		return AddTrack();
	return false;
}

bool TcxReader::NextPoint(Point &pnt)
{
	if (!oTrk)
		throw exception();

	if (oTrkPt)
		oTrkPt = oTrkPt->NextSiblingElement("Trackpoint");
	else
		oTrkPt = oTrk->FirstChildElement("Trackpoint");

	if (oTrkPt)
	{
		auto pos = oTrkPt->FirstChildElement("Position");
		auto ele = oTrkPt->FirstChildElement("AltitudeMeters");
		if (pos)
		{
			auto lat = pos->FirstChildElement("LatitudeDegrees");
			auto lon = pos->FirstChildElement("LongitudeDegrees");

			if (lat && lon)
			{
				if (lat->QueryDoubleText(&pnt.oLat))
					return false;
				if (lon->QueryDoubleText(&pnt.oLon))
					return false;

				pnt.oEle = 0.0;
				if (ele && ele->QueryDoubleText(&pnt.oEle))
					pnt.oEle = 0.0;
				return true;
			}
		}
	}

	return false;
}

/*******************************************************************/

CsvWriter::CsvWriter(Params &params)
: Writer(params)
{}

void CsvWriter::WritePoint(const Point &p)
{
	cout << fixed << setprecision(6)
		<< p.oLat << ","
		<< p.oLon << ","
		<< setprecision(1)
		<< p.oEle
		<< endl;
}

/*******************************************************************/

KmlWriter::KmlWriter(Params &params)
: Writer(params)
{
	//oStream.flags(ios_base::app | ios_base::out);
}

void KmlWriter::WritePoint(const Point &p)
{
	oStream << fixed << setprecision(6)
		<< p.oLon << ","
		<< p.oLat << ","
		<< setprecision(1)
		<< p.oEle << " ";
}

void KmlWriter::PreWrite()
{
	auto decl = oDoc.NewDeclaration(nullptr);
	auto kml = oDoc.NewElement("kml");
	oRoot = oDoc.NewElement("Document");

	kml->SetAttribute("xmlns", "http://www.opengis.net/kml/2.2");

	oDoc.InsertEndChild(decl);
	oDoc.InsertEndChild(kml);
	kml->InsertEndChild(oRoot);
}

void KmlWriter::PostWrite()
{
	if (oParams.oOutFilename.empty())
		oDoc.Print(nullptr);
	else
		oDoc.SaveFile(oParams.oOutFilename.c_str());
}

void KmlWriter::PreWritePoints()
{
	if (!oRoot)
		throw exception();

	auto mark = oDoc.NewElement("Placemark");
	auto name = oDoc.NewElement("name");
	auto line = oDoc.NewElement("LineString");
	oCoords =   oDoc.NewElement("coordinates");

	name->InsertEndChild(oDoc.NewText("Sample name"));

	oRoot->InsertEndChild(mark);
	mark->InsertEndChild(name);
	mark->InsertEndChild(line);
	line->InsertEndChild(oCoords);
}

void KmlWriter::PostWritePoints()
{
	if (!oCoords)
		throw exception();

	oCoords->InsertEndChild(oDoc.NewText(oStream.str().c_str()));
}

/*******************************************************************/

Reader *ReaderFactory::NewReader(FileType type, Params &params)
{
	switch (type)
	{
		case GPX:
			return new GpxReader(params);
		case TCX:
			return new TcxReader(params);
		default:
			return nullptr;
	}
}

/*******************************************************************/

Writer *WriterFactory::NewWriter(FileType type, Params &params)
{
	switch (type)
	{
		case CSV:
			return new CsvWriter(params);
		case KML:
			return new KmlWriter(params);
		default:
			return nullptr;
	}
}

/*******************************************************************/

bool Utils::FileExists(string filename)
{
	ifstream in(filename.c_str());
	return in;
}

FileType Utils::GetInFileType(string filename)
{
	XMLDocument doc;
	doc.LoadFile(filename.c_str());
	auto root = doc.RootElement();

	if (root)
	{
		map<string, FileType> rootNames = {
			{"gpx", GPX}, 
			{"TrainingCenterDatabase", TCX},
			{"kml", KML}
		};

		auto itr = rootNames.find(root->Value());
		if (itr != rootNames.end())
			return (*itr).second;
	}
	return UNKNOWN;
}

} // trkconv namespace