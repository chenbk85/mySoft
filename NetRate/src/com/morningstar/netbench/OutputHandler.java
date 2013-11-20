package com.morningstar.netbench;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;


class OutputHandler extends Thread {
	private Socket socket;
	private Completion status;
	OutputHandler(Socket socket, Completion status) {
		this.socket = socket;
		this.status = status;
	}
	
	public void run() {
		long count = 0, t1 = 0, t2;
		
		try {
			OutputStream out = socket.getOutputStream();
			byte[] buf = new byte[128*1024];

			t1 = System.currentTimeMillis();
			while (status.getStatus() == 0) {
				out.write(buf, 0, buf.length);
				count += buf.length;
			}
		}
		catch (IOException e) {
			System.err.print("IOException: ");
			e.printStackTrace();
		}
		
		t2 = System.currentTimeMillis() - t1;
		if (t2 < 1) t2 = 1;

		System.out.println(socket.toString() + " sent: " + count
				+ ", rate: " + ((double)count / 1000.0 / t2) + "MB/s");
	}
};
