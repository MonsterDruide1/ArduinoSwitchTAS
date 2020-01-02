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

	SerialPort serialPort;
	OutputStream outputStream;
	InputStream inputStream;
	Boolean serialPortOpen = false;
	
    public void run(){
        if (openSerialPort(portName) != true)
        	return;
    }
    
	boolean openSerialPort(String portName)
	{
		Boolean foundPort = false;
		if (serialPortOpen != false) {
			System.out.println("Serialport already open");
			return false;
		}
		System.out.println("Opening Serialport...");
		CommPortIdentifier serialPortId = null;
		Enumeration enumComm = CommPortIdentifier.getPortIdentifiers();
		while(enumComm.hasMoreElements()) {
			serialPortId = (CommPortIdentifier) enumComm.nextElement();
			if (portName.contentEquals(serialPortId.getName())) {
				foundPort = true;
				break;
			}
		}
		if (foundPort != true) {
			System.out.println("Serialport not found: " + portName);
			return false;
		}
		try {
			serialPort = (SerialPort) serialPortId.open("Öffnen und Senden", 500);
		} catch (PortInUseException e) {
			System.out.println("Port occupied");
		}
		try {
			outputStream = serialPort.getOutputStream();
		} catch (IOException e) {
			System.out.println("Can't access OutputStream");
		}

		try {
			inputStream = serialPort.getInputStream();
		} catch (IOException e) {
			System.out.println("Can't access InputStream");
		}
		try {
			serialPort.addEventListener(new serialPortEventListener());
		} catch (TooManyListenersException e) {
			System.out.println("TooManyListenersException for Serialport");
		}
		serialPort.notifyOnDataAvailable(true);

		try {
			serialPort.setSerialPortParams(115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
		} catch(UnsupportedCommOperationException e) {
			System.out.println("Couldn't set params");
		}
		
		serialPortOpen = true;
		System.out.println("DONE!");
		return true;
	}

	void closeSerialPort()
	{
		if (serialPortOpen == true) {
			System.out.println("Closing Serialport...");
			serialPort.close();
			serialPortOpen = false;
		} else {
			System.out.println("Serialport already closed");
		}
	}
	
	void sendSerialPort(String message)
	{
		if (serialPortOpen != true)
			return;
		try {
			outputStream.write(message.getBytes());
			outputStream.flush();
		} catch (IOException e) {
			System.out.println("Error while sending message");
		}
	}
	
	void readSerialPort() {
		try {
			byte[] data = new byte[150];
			int num;
			while(inputStream.available() > 0) {
				num = inputStream.read(data, 0, data.length);
				String received = new String(data, 0, num).trim();
				if(received.contains("1")) {
					System.out.println("Requested new entry: "+received);
					sendSerialPort(entries[i]+"\n");
					i++;
				}
				if(received.contains("START")) {
					i=0;
					System.out.println("RESET: "+received);
				}
				else {
					System.out.println("Receiving: "+ received);
				}
			}
		} catch (IOException e) {
			System.out.println("Error while reading received data");
		}
	}
	
	class serialPortEventListener implements SerialPortEventListener {
		public void serialEvent(SerialPortEvent event) {
			switch (event.getEventType()) {
			case SerialPortEvent.DATA_AVAILABLE:
				readSerialPort();
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