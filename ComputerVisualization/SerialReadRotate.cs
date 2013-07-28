using UnityEngine;
using System.Collections;
using System.IO.Ports;

public class SerialReadRotate : MonoBehaviour {
	
	public Quaternion rotation = Quaternion.identity;
	public string rec2;
	public int lastBow;
	public float smooth = 100.0F;

	public int accelX;
	public int accelY;
	public int accelZ;

	public int gyroX;
	public int gyroY;
	public int gyroZ;
	
	int gyroJam = 0;
	int gyroJamTwo = 0;
	Quaternion current = Quaternion.Euler(0, 0, 0);
	Quaternion currentTwo = Quaternion.Euler(0, 0, 0);

	SerialPort sp = new SerialPort("/dev/tty.usbmodemfd121", 115200);
	
	void Start ()
	{
		sp.Open();
		//sp.ReadTimeout = 10;
	}

	void Update ()
	{

		if(sp.IsOpen)
		{
			try
			{
				rec2 = (sp.ReadLine());
				
				if(rec2.Contains("start"))
				{
					accelX = int.Parse(sp.ReadLine ());
					accelY = int.Parse(sp.ReadLine ());
					accelZ = int.Parse(sp.ReadLine ());

					gyroX = int.Parse(sp.ReadLine ());
					gyroY = int.Parse(sp.ReadLine ());
					gyroZ = int.Parse(sp.ReadLine ());

					int elbow = int.Parse(sp.ReadLine ());
					
					if(gameObject.name == "piston_002")
					{				
						if(elbow >= lastBow + 20 || elbow <= lastBow - 20)
						{
							RotatePot(elbow);
						}
						lastBow = elbow;
					}
					else if(gameObject.name == "piston_001")
					{				
						RotateAccel();
					}
					else if(gameObject.name == "piston_000_X_Rot")
					{
						RotateGyro();
					}
				}
			}
			catch (System.Exception) {
				print("OHHH NOESSSS!!!!");
			}
		}
	}
	void RotatePot(int elbow)
	{
		Quaternion target = Quaternion.Euler(0, elbow/20, 0);
		Quaternion limit = Quaternion.Euler(0, 25, 0);

		if (elbow < 425*4)
		{
			transform.localRotation = Quaternion.Slerp(transform.rotation, target, Time.deltaTime*smooth);
		}
		else if (elbow >= 425*4)
		{
			transform.localRotation = Quaternion.Slerp(transform.rotation, limit, Time.deltaTime*smooth);
		}
	}

	void RotateAccel()
	{
		gyroJamTwo += gyroX/50;
		currentTwo = Quaternion.Euler(0, -gyroJamTwo, 0);
		transform.localRotation = Quaternion.Slerp(transform.rotation, currentTwo, Time.deltaTime*smooth);
	}

	void RotateGyro()
	{
	 	gyroJam += gyroY/50;
		current = Quaternion.Euler(0, -90, gyroJam);
		transform.localRotation = Quaternion.Slerp(transform.rotation, current, Time.deltaTime*smooth);
	}
	
}
