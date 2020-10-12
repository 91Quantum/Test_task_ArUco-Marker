using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System.Net.NetworkInformation;
using System.Collections.Specialized;
using System.Threading;
using System.Security.Cryptography;
using System;

public class CubeScript : MonoBehaviour
{
    // Impportierung der benötigten Funktionen aus der DLL-Datei. Der Name der DLL-Datei ist "OpenCV_Library".
    // Als Entrypoint werden die jeweiligen Funktionsnamen in der DLL verwendet. Kann mit dem Programm "Dependency Walker"
    // nachgeschaut werden.

    // Importierung der initialize()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "initialize")]
    public static extern void initialize(int cameraInput);

    // Importierung der loadCameraCalibration()-Funktion. Erwartet eigentlich ein const char*, die Korrespondenz in C#
    // dazu ist ein string!
    [DllImport("OpenCV_Library", EntryPoint = "loadCameraCalibration")]
    public static extern bool loadCameraCalibration(string cameraCalibrationFileName);

    // Importierung der estimatePoseMarkerAndDetection()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "estimatePoseMarkerAndDetection")]
    public static extern int estimatePoseMarkerAndDetection();

    // Importierung der getXCoordinate()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "getXCoordinate")]
    public static extern double getXCoordinate();

    // Importierung der getYCoordinate()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "getYCoordinate")]
    public static extern double getYCoordinate();

    // Importierung der getZCoordinate()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "getZCoordinate")]
    public static extern double getZCoordinate();

    // Importierung der close()-Funktion
    [DllImport("OpenCV_Library", EntryPoint = "close")]
    public static extern void close();

    // Erstellung eines Rigidbody, um die Kontrolle der Position des Würfels zu bekommen/ beeinflussen
    public Rigidbody rb;
  


    // Start wird vor dem ersten Frameupdate aufgerufen
    void Start()
    {
        rb = GetComponent<Rigidbody>();

        // Wenn isKinematic aktiv ist, Kräfte, Kollisionen oder Joints haben keinen Einfluss auf den Rigidbody
        rb.isKinematic = true;

        /*initialize() - Funktion: Initialisierung wichtiger Objekte, zur Durchführung der Prozesse
                -@param 0: Kamerainput als Integer-Wert (0 als Standardanschluss für eine angeschlossene Kamera) */
        initialize(0);

        /* loadCameraCalibration()-Funktion: Laden der Kamerakalibrierungsdaten aus einer Datei
		        - @param "CameraCalibration": Name der zu ladenen Datei
		        - @param return: True oder false, ob der Ladevorgang erfolgen konnte oder nicht*/
        loadCameraCalibration("CameraCalibration");
        Debug.Log("Camera Calibration loaded: " + loadCameraCalibration("CameraCalibration"));
       
    }



    // Wenn die Anwendung beendet wird, werden alle Fenster geschlossen und nicht mehr benötigte Objekte "freigegeben"
    void OnApplicationQuit()
    {
        close();
    }



    // Update wird einmal pro Frame aufgerufen
    void Update()
    {
        /* estimatePoseMarkerAndDetection()-Funktion: Durchführung der Posenschätzung der Marker und deren Erkennung
		        - @param return: -1 für einen Fehlschlag, 1 für eine Durchführung*/
        estimatePoseMarkerAndDetection();

        // Übertragung der Translations-Koordinaten. Dabei werden die double-Koordinaten in float umgewandelt
        // Die Y-Koordinate wird mit (-1) multipliziert, weil OpenCv ein rechtshändiges Koordinatensystem hat
        // und Unity ein linkshändiges Koordinatensystem verwendet
        transform.position = new Vector3(Convert.ToSingle(getXCoordinate()), Convert.ToSingle(getYCoordinate()) * (-1.0f),
            Convert.ToSingle(getZCoordinate()));
        rb.MovePosition(transform.position * Time.deltaTime);

        //Debug.Log("X-Translation: " + Convert.ToSingle(getXCoordinate()));
        //Debug.Log("Y-Translation: " + Convert.ToSingle(getYCoordinate()));
        //Debug.Log("Z-Translation: " + Convert.ToSingle(getZCoordinate()));

    }

}
