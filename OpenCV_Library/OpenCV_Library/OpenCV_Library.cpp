#include "pch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

using namespace std;
using namespace cv;

// extern "C" verhindert, dass Funktionen, die in C++ definiert sind, nicht "verändert" werden (mangling), so dass sie von C aus
// aufgerufen werden können. Mit dem Schlüsselwort __declspec(dllexport) können Daten, Funktionen, Klassen oder Klassenmemberfunktionen
// aus einer DLL exportiert werden.
extern "C" __declspec(dllexport) void initialize(int);
extern "C" __declspec(dllexport) bool loadCameraCalibration(const char*);
extern "C" __declspec(dllexport) int estimatePoseMarkerAndDetection();
extern "C" __declspec(dllexport) double getXCoordinate();
extern "C" __declspec(dllexport) double getYCoordinate();
extern "C" __declspec(dllexport) double getZCoordinate();
extern "C" __declspec(dllexport) void close();

// Kameramatrix (intrinsische Matrix) 3x3
Mat cameraMatrix;

// Abstandskoeffizienten 5X1
Mat distanceCoefficients;

// Länge des ArUco-Markers aus dem Lexikon in Metern DICT_4X4_50
const float arucoSquareDimension = 0.132f;

// Pointer für die Webcam
Ptr<VideoCapture> cap;

// Array für das Aufnahmebild der Webcam
Mat frame;

// ArUco-Lexikon-Objekt
Ptr<aruco::Dictionary> dictionary;

// Vektor für die erkannten Marker-IDs
vector<int> markerIds;

// "2-dimensionales Arrray" für die Merkerecken und die abgelehnten Kandidaten
vector<vector<Point2f>> markerCorners, rejectedCandidates;

// Paramter-Objekt, das eventuell genutzt werden kann
aruco::DetectorParameters parameters;

// "2-dimensionales Array" für die Rotationen und Translationen
vector<Vec3d> rotationVectors, translationVectors;


/* initialize()-Funktion: Initialisierung wichtiger Objekte, zur Durchführung der Prozesse
		- @param cameraInput: Kamerainput als Integer-Wert (0 als Standard für eine angeschlossene Kamera)*/
void initialize(int cameraInput) {

	// Kameramatrix als 3x3
	cameraMatrix = Mat::eye(3, 3, CV_64F);

	cap = new VideoCapture(cameraInput);

	// Erstellung des verwendeten Lexikons der ArUco-Marker (hier: DICT_4X4_50)
	dictionary = aruco::getPredefinedDictionary(aruco::DICT_4X4_50);
}


/* loadCameraCalibration()-Funktion: Laden der Kamerakalibrierungsdaten aus einer Datei
		- @param cameraCalibrationFileName: Name der zu ladenen Datei (const char* für C-Übersetzung)
		- @param return: True oder false, ob der Ladevorgang erfolgen konnte oder nicht*/
bool loadCameraCalibration(const char* cameraCalibrationFileName) {

	// Erstellung eines ifstream, um Daten aus einer Datei zu lesen
	ifstream inStream(cameraCalibrationFileName);

	if (inStream) {

		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		// 3x3 Kameramatrix
		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; ++r) {

			for (int c = 0; c < columns; ++c) {

				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";
			}
		}

		inStream >> rows;
		inStream >> columns;

		// 5x1 Abstandskoeffizienten-Matrix
		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; ++r) {

			for (int c = 0; c < columns; ++c) {

				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}

		inStream.close();
		return true;
	}

	return false;
}


/* estimatePoseMarkerAndDetection()-Funktion: Durchführung der Posenschätzung der Marker und deren Erkennung
		- @param return: -1 für einen Fehlschlag, 1 für eine Durchführung*/
int estimatePoseMarkerAndDetection() {

	// Wenn die Webcam (cap) nicht geöffnet werden kann, dann return -1
	if (!cap->isOpened()) {

		return -1;
	}

	// Falls das Videobild der Kamera nicht gelesen werden kann, dann return -1
	if (!cap->read(frame)) {

		return -1;
	}

	/* detectMarkers()-Funktion: Grundlegende Markererkennung
				- @param frame: Eingabebild (Webcam)
				- @param dictionary: Gibt die Art der Marker an, die durchsucht werden sollen (hier: DICT_4X4_50)
				- @param markerCorners: Vektor der erkannten Marker-Ecken. Für N erkannte Marker sind die Dimensionen des Arrays Nx4
				- @param markerIds: Vektor der Identifikationen der erkannten Markierungen. Für N erkannte Marker ist die Dimension
									des Arrays N*/
	aruco::detectMarkers(frame, dictionary, markerCorners, markerIds);


	/* estimatePoseSingleMarkers()-Funktion: Posenschätzung für einzelne Marker
				- @param markerCorners: Vektor der bereits erkannten Markerecken
				- @param arucoSquareDimension: Länge der ArUco-Markers. Die Translationvektoren werden normalerweise in derselben
											   Einheit ausgegeben -> Metern
				- @param cameraMatrix: Die zuvor bestimmte intrinsische Kameramatrix
				- @param distanceCoefficients: Vektor der zuvor bestimmten Abstandskoeffizienten
				- @param rotationVectors: Ausgabearray von Rotationsvektoren
				- @param translationVectors: Ausgabearray von Translationsvektoren. Jedes Element in tvecs entspricht der
											 spezifischen Markierung in imgPoints
		   Für jeden Marker wird ein Translations- und ein Rotationsvektor ausgegeben.*/
	aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix,
		distanceCoefficients, rotationVectors, translationVectors);

	// Wenn ein Marker erkannt worden ist, zeichne den erkannten Marker
	if (markerIds.size() > 0) {

		/* drawDetectedMarkers()-Funktion: Zeichnet die erkannten Bilder im Bild (Webcam-frame)
				- @param frame: Eingabebild der Kamera
				- @param markerCorners: Die erkannten Markerecken im Eingabebild. Für N erkannte Marker, sind die Dimensionen
										des Arrays Nx4
				- @param markerIds: Vektor von Bezeichnern für Marker (optional)*/
		aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
	}

	for (int i = 0; i < markerIds.size(); ++i) {

		/* drawAxis()-Funktion: Zeichnet die Achsen des Koordinatensystem aus der Posenschätzung
					- @param frame: Eingabebild (Webcam)
					- @param cameraMatrix: Zuvor bestimmte intrinsische Kameramatrix
					- @param distanceCoefficients: Die vorher bestimmten Abstandskoeffizienten
					- @param rotationVectors[i]: Rotationsvektoren der Koordinatensysteme, die gezeichnet werden
					- @param translationVectors[i]: Translationsvektoren der Koordinatensysteme, die gezeichnet werden
					- @param 0.1f: Länge der gemalten Achsen, in der gleichen Einheit wit tVec (Normalerweise in Metern)*/
		aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
	}

	/* imshow()-Funktion: Stellt ein Bild in einem spezifischen Fenster dar
				- @param "Webcam": Stellt den Fensternamen dar
				- @param frame: Darzustellende Bild
				Auf dieser Funktion sollte immer die waitKey()-Funktion folgen, weil sonst das Fenster nicht dargestellt wird*/
	imshow("Webcam", frame);

	return 1;
}

/* getXCoordinate()-Funktion: Gibt die X-Koordinate des Markers wieder in Metern (aus seinem tVec!)
		- @param return: X-Koordinate des ersten erkanten Markers*/
double getXCoordinate() {

	// Falls der tVec nicht leer ist (Kein Marker erkannt)
	if (!translationVectors.empty()) {

		return translationVectors[0][0];
	}

	return 0.0;
}


/* getYCoordinate()-Funktion: Gibt die Y-Koordinate des Markers wieder in Metern (aus seinem tVec!)
		- @param return: Y-Koordinate des ersten erkanten Markers*/
double getYCoordinate() {

	// Falls der tVec nicht leer ist (Kein Marker erkannt)
	if (!translationVectors.empty()) {

		return translationVectors[0][1];
	}

	return 0.0;
}


/* getZCoordinate()-Funktion: Gibt die Z-Koordinate des Markers wieder in Metern (aus seinem tVec!)
		- @param return: Z-Koordinate des ersten erkanten Markers*/
double getZCoordinate() {

	// Falls der tVec nicht leer ist (Kein Marker erkannt)
	if (!translationVectors.empty()) {

		return translationVectors[0][2];
	}

	return 0.0;
}

/* close()-Funktion: Schließt Fenster und "befreit" einige Objekte*/
void close() {

	cameraMatrix.release();
	distanceCoefficients.release();
	frame.release();
	dictionary.release();
	destroyAllWindows();
}
