package com.morningstar.netbench;

import java.io.*;
import java.net.*;

public class nrClient {

	public static void main(String[] args) {
		String host = (args.length > 0) ? args[0] : "localhost";
		int port = (args.length > 1) ? Integer.parseInt(args[1]) : 9080;

		try {
			Socket socket = new Socket(host, port);
			Completion status = new Completion(0);

			InputHandler ih = new InputHandler(socket, status);
			OutputHandler oh = new OutputHandler(socket, status);
			
			ih.start();
			oh.start();

			Thread.sleep(10*1000);

			status.setStatus(1); ih.join();
			status.setStatus(1); oh.join();
		}
		catch (UnknownHostException e) {
			System.err.print("UnknownHostException: ");
			e.printStackTrace();
		}
		catch (IOException e) {
			System.err.print("IOException: ");
			e.printStackTrace();
		}
		catch (InterruptedException e) {
			System.err.print("InterruptedException: ");
			e.printStackTrace();
		}
		
		System.out.println("OK?");
	}
};
