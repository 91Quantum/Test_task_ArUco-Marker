# Test_task_ArUco-Marker
The aim of the test task is, that a printed ArUco marker should be captured by a camera connected to the computer and the relative movements of this marker should also move a virtual object (e.g. cube) in Unity3D according to the marker movement.

The folder "OpenCV_Calibration" contains the whole processes of:
  - Create ArUco marker from a dictionary
  - The calibration of the camera with a chessboard
  - The pose estimation of the ArUco markers
  
The "OpenCV_Library" folder contains the DLL with the interfaces to obtain the coordinates from the detected ArUco marker in the scene.They are needed to move the cube
in the Unity scene.

The Assets folder contains the important materials for the scene, the DLL and the Script for the Cube. The script calls the functions in the DLL, to get the 3D coordinates of the ArUco marker and transmit the informations to the movement of the cube.
