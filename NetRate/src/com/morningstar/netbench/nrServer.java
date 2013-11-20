package com.morningstar.netbench;

import java.io.*;
import java.net.*;

public class nrServer {
	public static void main(String[] args) {
		ServerSocket server;
		Socket child;
		int port = (args.length > 0) ? Integer.parseInt(args[0]) : 9080;
		
		try {
			server = new ServerSocket(port);
			System.out.println("server is listening at: " + server.getLocalSocketAddress());
			while ((child = server.accept()) != null) {
				System.out.println("new socket: "
						+ child.getRemoteSocketAddress()
						+ " ---> "
						+ child.getLocalSocketAddress());

				new InputHandler(child, new Completion(0)).start();
				new OutputHandler(child, new Completion(0)).start();
			}
		}
		catch (UnknownHostException e) {
			System.err.print("UnknownHostException: ");
			e.printStackTrace();
		}
		catch (IOException e) {
			System.err.print("IOException: ");
			e.printStackTrace();
		}
	}
};
