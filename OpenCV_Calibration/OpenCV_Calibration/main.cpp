#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

// Länge eines Quadrats des Chessboards in Metern
const float calibrationSquareDimension = 0.0245f;

// Länge des ausgedruckten ArUco-Markers (hier: DICT_4X4_50) in Metern
const float arucoSquareDimension = 0.132f;

// Dimensionen des Chessboards (hier: 9, 6 -> Kreuzungen der schwarzen und weißen Quadraten (Spalten, Reihen))
const Size chessboardDimensions = Size(9, 6);



/* createArucoMarker() - Funktion: Generierung von Aruco-Markers aus einer bestimmten Lexikon*/
void createArucoMarkers() {

	//Matrix (Array) für das Bild des Markers
	Mat outputMarker;

	/*Erstellung des Lexikons [DICT_(bits)x(bits)_(Anzahl Marker)]:
			- DICT_4X4_50
			- DICT_4X4_100
			- DICT_4X4_250
			- DICT_4X4_1000
			- DICT_5X5_50
			- DICT_5X5_100
			- DICT_5X5_250
			- DICT_5X5_1000
			- DICT_6X6_50
			- DICT_6X6_100
			- DICT_6X6_250
			- DICT_6X6_1000
			- DICT_7X7_50
			- DICT_7X7_100
			- DICT_7X7_250
			- DICT_7X7_1000
		 */
	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);


	for (int i = 0; i < 50; ++i) {

		/* darwMarker()-Funktion: Marker die in outputMarker gespeichert werden
			 - @param markerDictionary: Zuvor angelegte Dictionary-Objekt, welches übergeben wird
			 - @param i:  Marker-ID
			 - @param 500: ist die Größe des Ausgabemarkierungsbildes, in diesem Fall wird das Ausgabebild eine Größe von 500x500 Pixel haben.
			   Der Parameter sollte groß genug gewählt werden, um die Anzahl der Bits für das spezifische Lexikon zu speichern
			 - @param outputMarker: n-dimensionales Array in dem die Marker gespeichert werden
			 - @param 1: Optionaler Parameter zur Angabe der Breite des schwarzen Rands der Markierung.
			   Die Größe wird proportional zur Anzahl der Bits angegeben. Zum Beispiel bedeutet ein Wert von 2, dass der Rand 
			   eine Breite hat, die der Größe von zwei internen Bits entspricht. Der Standardwert ist 1*/
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
		
		//Ausgabe der Marker als .jpg Datei
		ostringstream convert;
		string imageName = "4x4Marker_";
		convert << imageName << i << ".jpg";

		/* imwrite()-Funktion: Speichert das Bild in eine spezifische Datei
			 - @param convert.str(): Name der Datei
			 - @param outputMarker:  Marker die gespeichert werden sollen*/
		imwrite(convert.str(), outputMarker);
	}
}



/* createKnownBoardPositions()-Funktion: Erstellung der Eckpositionen auf dem Chessboard
	- @param boardSize: Größe des Chessboards (hier: 9, 6)
	- @param squareEdgeLength: Länge eines Quadrates auf dem Chessboard
	- @param corners: Vektor in dem die bestimmten Eckpositionen gespeichert werden (hier: Z = 0, da flache Fläche)*/
void createKnownBoardPositions(Size boardSize, float squareEdgeLength, vector<Point3f>& corners) {

	for (int i = 0; i < boardSize.height; ++i) {

		for (int j = 0; j < boardSize.width; ++j) {

			corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
		}
	}
}



// Methode über individuell, gespeicherte Bilder
/* getChessboardCorners()-Funktion: Finde und visualiesiere die gefundenen Eckpositionen (Kreuzungspositionen)
	- @param images: Reihe von Bildern die übergeben werden sollen, um die Eckpositionen zu bestimmen
	- @param allFoundCorners: Alle gefundenen Ecken, die ausgegeben werden sollen
	- @param showResults: Zeige die Ergebnisse der gefundenen Ecken*/
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false) {

	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); ++iter) {

		// Buffer der alle Punkte enthält, die im Bild (images) gefunden worden sind
		vector<Point2f> pointBuf;

		/*findChessboardCorners()-Funktion: Findet die Positionen der inneren Ecken des Chessboards
			- @param *iter: Bild das untersucht werden soll
			- @param Size(9, 6): Anzahl der Innenecken pro Schachbrettreihe und -spalte (Size(columns, rows))
			- @param pointBuf: Ausgabearray der erkannten Ecken
			- @param CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE: Verschiedenen Operationsflags
			- @output: true oder false, ob die Innenekcen gefunden werden konnten*/
		bool found = findChessboardCorners(*iter, Size(9, 6), pointBuf, 
			CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);

		if (found) {

			allFoundCorners.push_back(pointBuf);
		}

		if (showResults) {

			/* drawChessboardCorners()-Funktion: Rendert die erkannten Chessboard-Ecken
				- @param *iter: Zielbild, auf dem die Ecken gerendert werden sollen
				- @param Size(9, 6): Anzahl der Innenecken pro Schachbrettreihe und -spalte (Size(columns, rows))
				- @param pointBuf: Array der erkannten Ecken (Ausgabe von s. findChessboardCorners())
				- @param found: Parameter, der angibt, ob das komplette Board gefunden wurde oder nicht*/
			drawChessboardCorners(*iter, Size(9, 6), pointBuf, found);

			/* imshow()-Funktion: Stellt ein Bild in einem spezifischen Fenster dar
				- @param "Looking for Corners": Stellt den Fensternamen dar
				- @param *iter: Darzustellende Bild
				Auf dieser Funktion sollte immer die waitKey()-Funktion folgen, weil sonst das Fenster nicht dargestellt wird*/
			imshow("Looking for Corners", *iter);
			
			/* waitKey()-Funktion: Wartet so lange bis eine Taste gedrückt wird oder eine bestimmte Zeit vergangen ist
				 - @param 0: Gibt den Delay in Millisekunden an (0 meint für immer)*/
			waitKey(0);
		}

	}
}




/* cameraCalibration()-Funktion: Durchführung der Kamerakalibireung (Erstellung der 3x3-Kameramatrix und Abstandskoeffizienten)
	- @param calibrationImages: Es werden Bilder für eine Kalibrieung übergeben, die bereits als gültig erkannt worden sind
	- @param boardSize: Anzahl der Innenecken pro Schachbrettreihe und -spalte (Size(columns, rows))
	- @param squareEdgeLength: Länge eines Quadrates auf dem Chessboard
	- @param cameraMatrix: 3x3-Kameramatrix, die übergeben wird, um die Werte der Kalibireung zu speichern
	- @param distanceCoefficients: Abstandskoeffizienten, die ebenfalls übergebn werden, um sie zu bestimmen*/
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, 
	Mat& cameraMatrix, Mat& distanceCoefficients) {

	// Punkte der Ecken, die wir auf den Bildern erkennen
	vector<vector<Point2f>> checkerboardImageSpacePoints;

	/* getChessboardCorners()-Funktion: Finde und visualiesiere die gefundenen Eckpositionen (Kreuzungspositionen)
	- @param calibrationImages: Reihe von Bildern die übergeben werden sollen, um die Eckpositionen zu bestimmen
	- @param checkerboardImageSpacePoints: Alle gefundenen Ecken, die ausgegeben werden sollen
	- @param false: Zeige die Ergebnisse der gefundenen Ecken*/
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	//Bekannte Board-Positionen mit der Größe 1
	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	/* createKnownBoardPositions()-Funktion: Erstellung der Eckpositionen auf dem Chessboard
	- @param boardSize: Größe des Chessboards (hier: 9, 6)
	- @param squareEdgeLength: Länge eines Quadrates auf dem Chessboard
	- @param worldSpaceCornerPoints[0] (erstes Element): Vektor in dem die bestimmten Eckpositionen gespeichert werden*/
	createKnownBoardPositions(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
	
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);


	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	/* calibrateCamera()-Funktion: Findet die kamerainternen und -externen Parameter aus mehreren Ansichten eines Kalibriemusters
		- @param worldSpaceCornerPoints: 
		- @param checkerboardImageSpacePoints:
		- @param boardSize: Größe des Bildes, das zur Initialisierung der kamerinternen Matrix verwendet wird (hier: 9, 6)
		- @param cameraMatrix: 3x3 Kamera-Matrix (intrinsische Matrix)
		- @param distanceCoefficients: Vektor der Abstandskoeffizienten
		- @param rVectors: Output der Rotationsvektoren
		- @param tVectors: Output der Translationsvektoren*/
	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, 
		distanceCoefficients, rVectors, tVectors);
}


/* saveCameraCalibration()-Funktion: Speicherung der Kamerakalibrierungsdaten als Datei
		- @param name: Name der zu speichernden Datei
		- @param cameraMatrix: Matrix der intrinsischen Kameradaten
		- @param distanceCoefficients: Daten der Abstandskoeffizienten
		- @param return: True oder false, ob die Speicherung erfolgen konnte oder nicht*/
bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients) {

	//Erstellung eines ofstream, um mit Daten arbeiten zu können
	ofstream outStream(name);

	if (outStream) {

		// 3x3 Kameramatrix
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; ++r) {

			for (int c = 0; c < columns; ++c) {

				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;

			}
		}

		// 5x1 Abstandskoeffizienten-Matrix
		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; ++r) {

			for (int c = 0; c < columns; ++c) {

				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;

			}
		}

		outStream.close();
		return true;
	}

	return false;
}



/* loadCameraCalibration()-Funktion: Laden der Kamerakalibrierungsdaten aus einer Datei
		- @param name: Name der zu ladenen Datei
		- @param cameraMatrix: Kameramatrix in dem die intrinsischen daten aus der Datei gespeichert werden
		- @param distanceCoefficients: Abstandskoeffizienten-Matrix in dem die Daten aus der Datei geladen werden
		- @param return: True oder false, ob der Ladevorgang erfolgen konnte oder nicht*/
bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients) {

	// Erstellung eines ifstream, um Daten aus einer Datei zu lesen
	ifstream inStream(name);

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



/* startWebcamMonitoring()-Funktion: Posenschätzung der ArUco-Marker
		- @param cameraMatrix: Intrinsische Kameramatrix, die vorher erstellt werden muss
		- @param distanceCoefficients: Abstandskoeffizienten, die vorher bestimmt werden müssen
		- @param arucoSquareDimensions: Länge des ArUco-Markers*/
int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions) {

	Mat frame;

	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	VideoCapture vid(0);

	if (!vid.isOpened()) {

		return -1;
	}

	namedWindow("Webcam", WINDOW_AUTOSIZE);

	vector<Vec3d> rotationVectors, translationVectors;
	Vec3d translationToPosition;

	while (true) {

		if (!vid.read(frame)) {

			break;
		}

		/* detectMarkers()-Funktion: Grundlegende Markererkennung
				- @param frame: Eingabebild (Webcam)
				- @param markerDictionary: Gibt die Art der Marker an, die durchsucht werden sollen
				- @param markerCorners: Vektor der erkannten Marker-Ecken. Für N erkannte Marker sind die Dimensionen des Arrays Nx4
				- @param markerIds: Vektor der Identifikationen der erkannten Markierungen. Für N erkannte Marker ist die Dimension
									des Arrays N*/
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);

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
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, 
			rotationVectors, translationVectors);


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

		//Position des Markers im Kamerabild (Oben links Ausgangspunkt (0, 0, 0) Einheit in Meter [m])
		if (!translationVectors.empty()) {

			cout << "x-Koordinaten: " << translationVectors[0][0] << "m" << endl;
			cout << "y-Koordinaten: " << translationVectors[0][1] << "m" << endl;
			cout << "z-Koordinaten: " << translationVectors[0][2] << "m" << endl;
			cout << endl;
		}
			
		imshow("Webcam", frame);

		if (waitKey(30) >= 0) {

			break;
		}

	}

	return 1;
}


// Live Calibration Images Methode
/* cameraCalibrationProcess()-Funktion: Erstellung der 3x3 Kamerakalibrierungs-Matrix und der Abstandskoeffizienten
	- @param cameraMatrix: 3x3 Kamera-Matrix, die übergebn wird, um die Werte zu bestimmen
	- @param distnaceCoefficients: Array in welches die distance coefficients gespeichert werden*/
void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoefficients) {

	// frame beinhaltet die Videoinformationen der Webcam
	Mat frame;
	Mat drawToFrame;

	// Array in dem eine gute Kalibrierung gespeichert wird, über die Webcam
	vector<Mat> savedImages;

	// Gefundene Innenecken und abgelehnte Kandidaten
	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	// Erstellung der Videoquelle (hier: Quelle 0)
	VideoCapture vid(0);

	// Wenn die Quelle nicht offen ist, wird returnt
	if (!vid.isOpened()) {

		return;
	}


	int framePerSecond = 20;

	/* namedWindow()-Funktion: Erstellt ein Fenster
		- @param "Webcam": Fenstername
		- @param WINDOW_AUTOSIZE: Flag für das Fenster
			WINDOW_AUTOSIZE passt die Fenstergröße automatisch an das Bild an. Die Fenstergröße kann nicht manuell geändert werden*/
	namedWindow("Webcam", WINDOW_AUTOSIZE);

	while (true) {

		// Wenn das Bild der Webcam (frame) nicht gelesen werden kann, wird die Schleife beendet
		if (!vid.read(frame)) {
			break;
		}

		vector<Vec2f> foundPoints;
		bool found = false;

		/*findChessboardCorners()-Funktion: Findet die Positionen der inneren Ecken des Chessboards
			- @param *frame: Bild das untersucht werden soll (Webcam)
			- @param chessboardDimensions: Konstante, globale Variable für die Anzahl der Innenecken 
					 pro Schachbrettreihe und -spalte (Size(columns, rows))
			- @param foundPoints: Ausgabearray der erkannten Ecken
			- @param CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE: Verschiedenen Operationsflags
			- @output: true oder false, ob die Innenekcen gefunden werden konnten*/
		found = findChessboardCorners(frame, chessboardDimensions, foundPoints,
			CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
		
		// Kopiere das Videobild (frame), weil wir darauf zeichnen wollen
		frame.copyTo(drawToFrame);

		/* drawChessboardCorners()-Funktion: Rendert die erkannten Chessboard-Ecken
				- @param drawToFrame: Zielbild, auf dem die Ecken gerendert werden sollen (Webcam)
				- @param chessboardDimensions: Konstante, globale Variable für die Anzahl der Innenecken 
					 pro Schachbrettreihe und -spalte (Size(columns, rows))
				- @param foundPoints: Array der erkannten Ecken (Ausgabe von s. findChessboardCorners())
				- @param found: Parameter, der angibt, ob das komplette Board gefunden wurde oder nicht*/
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);

		// Wenn die Ecken gefunden worden sind, dann zeige sie in einem Fenster
		if (found) {

			imshow("Webcam", drawToFrame);
		}
		//Ansonsten zeige das Webcam-Videobild
		else {

			imshow("Webcam", frame);
		}

		char character = waitKey(1000 / framePerSecond);

		// Was während des Prozesses gemacht werden kann, abhängig welche Taste betätigt wird
		switch (character) {

		//Leertaste: Wird die Leertatse gedrückt, wird das Bild (frame) gespeichert
		case ' ':
			
			// Wenn die Ecken gefunden worden sind, dann wird es zu dem Array savedImages zwischengespeichert
			if (found) {

				Mat temp;

				frame.copyTo(temp);
				savedImages.push_back(temp);

				cout <<  "Anzahl Bilder: " << savedImages.size() << endl;
			}

			break;

		// Enter: Wird die Enter-Taste gedrückt, wird die Kalibrierung der Kamera gestartet. Dabei werden die Bilder verwendet,
		//		  die in savedImages zwischengespeichert worden sind
		case 13:
			
			//Es sollten mehr als 15 Bilder vorhanden sein, für die Kalibrierung
			if (savedImages.size() > 15) {

				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoefficients);
				saveCameraCalibration("CameraCalibration", cameraMatrix, distanceCoefficients);

				cout << "Kamerakalibrierung gespeichert!" << endl;
			}

			break;

		// Escape: Der ganze Prozess wird beendet
		case 27:

			cout << "Webcam wird geschlossen!" << endl;
			return;
			break;
		}
	}
}

int main(int argv, char** argc) {

	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);

	Mat distanceCoefficients;

	// 1.) Bilder machen, um die Koeffizienten zu bekommen, die gespeichert werden
	//     Leertaste: Bild machen; Enter: Kalibrieung starten (min. 15 Bilder); Escape: Exit
	//cameraCalibrationProcess(cameraMatrix, distanceCoefficients);
	
	// 2.) Kamerakalibrieung laden und Markerpositionen bestimmen/ anzeigen
	loadCameraCalibration("CameraCalibration", cameraMatrix, distanceCoefficients);
	startWebcamMonitoring(cameraMatrix, distanceCoefficients, arucoSquareDimension);

	return 0;
}