package paket;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.TooManyListenersException;

import javax.swing.JFileChooser;
import javax.swing.UIManager;

import gnu.io.CommPortIdentifier;
import gnu.io.PortInUseException;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;
import gnu.io.UnsupportedCommOperationException;

public class Main implements Runnable {

	String portName = "COM4"; //FIXME
	
	
	
	static int i = 0;
	static String[] entries;
	
	public static void main(String[] args)
	{
		try {
			entries = readFile(selectFile());
			Runnable runnable = new Main();
			new Thread(runnable).start();
			System.out.println("main finished");
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public static File selectFile() {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
			JFileChooser chooser = new JFileChooser();
			chooser.setCurrentDirectory(new File("D://Eigene_Dateien//Downloads//TAS"));
			chooser.showOpenDialog(null);
			UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
			return chooser.getSelectedFile();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}
	
	public static String[] readFile(File file) throws IOException {
		ArrayList<String> list = new ArrayList<String>();
		BufferedReader br = new BufferedReader(new FileReader(file));
		String line = "";
		while((line=br.readLine())!=null) {
			list.add(line);
		}
		br.close();
		return list.toArray(new String[0]);
	}

	CommPortIdentifier serialPortId;
	Enumeration enumComm;
	SerialPort serialPort;
	OutputStream outputStream;
	InputStream inputStream;
	Boolean serialPortGeoeffnet = false;
	
    public void run(){
        if (oeffneSerialPort(portName) != true)
        	return;
    }
    
	boolean oeffneSerialPort(String portName)
	{
		Boolean foundPort = false;
		if (serialPortGeoeffnet != false) {
			System.out.println("Serialport bereits geöffnet");
			return false;
		}
		System.out.println("Öffne Serialport");
		enumComm = CommPortIdentifier.getPortIdentifiers();
		while(enumComm.hasMoreElements()) {
			serialPortId = (CommPortIdentifier) enumComm.nextElement();
			if (portName.contentEquals(serialPortId.getName())) {
				foundPort = true;
				break;
			}
		}
		if (foundPort != true) {
			System.out.println("Serialport nicht gefunden: " + portName);
			return false;
		}
		try {
			serialPort = (SerialPort) serialPortId.open("Öffnen und Senden", 500);
		} catch (PortInUseException e) {
			System.out.println("Port belegt");
		}
		try {
			outputStream = serialPort.getOutputStream();
		} catch (IOException e) {
			System.out.println("Keinen Zugriff auf OutputStream");
		}

		try {
			inputStream = serialPort.getInputStream();
		} catch (IOException e) {
			System.out.println("Keinen Zugriff auf InputStream");
		}
		try {
			serialPort.addEventListener(new serialPortEventListener());
		} catch (TooManyListenersException e) {
			System.out.println("TooManyListenersException für Serialport");
		}
		serialPort.notifyOnDataAvailable(true);

		try {
			serialPort.setSerialPortParams(115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
		} catch(UnsupportedCommOperationException e) {
			System.out.println("Konnte Schnittstellen-Paramter nicht setzen");
		}
		
		serialPortGeoeffnet = true;
		System.out.println("DONE!");
		return true;
	}

	void schliesseSerialPort()
	{
		if ( serialPortGeoeffnet == true) {
			System.out.println("Schließe Serialport");
			serialPort.close();
			serialPortGeoeffnet = false;
		} else {
			System.out.println("Serialport bereits geschlossen");
		}
	}
	
	void sendeSerialPort(String nachricht)
	{
		if (serialPortGeoeffnet != true)
			return;
		try {
			outputStream.write(nachricht.getBytes());
			outputStream.flush();
		} catch (IOException e) {
			System.out.println("Fehler beim Senden");
		}
	}
	
	void serialPortDatenVerfuegbar() {
		try {
			byte[] data = new byte[150];
			int num;
			while(inputStream.available() > 0) {
				num = inputStream.read(data, 0, data.length);
				String received = new String(data, 0, num).trim();
				if(received.contains("1")) {
					System.out.println("Requested new entry: "+received);
					sendeSerialPort(entries[i]+"\n");
					i++;
				}
				if(received.contains("START")) {
					i=0;
					System.out.println("RESET: "+received);
				}
				else {
					System.out.println("Empfange: "+ received);
				}
			}
		} catch (IOException e) {
			System.out.println("Fehler beim Lesen empfangener Daten");
		}
	}
	
	class serialPortEventListener implements SerialPortEventListener {
		public void serialEvent(SerialPortEvent event) {
			switch (event.getEventType()) {
			case SerialPortEvent.DATA_AVAILABLE:
				serialPortDatenVerfuegbar();
				break;
			case SerialPortEvent.BI:
			case SerialPortEvent.CD:
			case SerialPortEvent.CTS:
			case SerialPortEvent.DSR:
			case SerialPortEvent.FE:
			case SerialPortEvent.OUTPUT_BUFFER_EMPTY:
			case SerialPortEvent.PE:
			case SerialPortEvent.RI:
			default:
				System.out.println("NOT DATA_AVAILABLE");
			}
		}
	}	
}