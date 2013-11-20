package com.morningstar.netbench;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;

class InputHandler extends Thread {
	private Socket socket;
	private Completion status;
	InputHandler(Socket socket, Completion status) {
		this.socket = socket;
		this.status = status;
	}
	
	public void run() {
		long count = 0, t1 = 0, t2;

		try {
			InputStream in = socket.getInputStream();
			byte[] buf = new byte[128*1024];
			
			t1 = System.currentTimeMillis();
			int rlen;
			while (status.getStatus() == 0 && (rlen = in.read(buf, 0, buf.length)) > 0) {
				count += rlen;
			}
		}
		catch (IOException e) {
			System.err.print("IOException: ");
			e.printStackTrace();
		}
		
		t2 = System.currentTimeMillis() - t1;
		if (t2 < 1) t2 = 1;

		System.out.println(socket.toString() + " received: " + count
				+ ", rate: " + ((double)count / 1000.0 / t2) + "MB/s");
	}
};
