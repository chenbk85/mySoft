package com.morningstar.netbench;

public class Completion {
	private int status;

	Completion() {
		this(0);
	}

	Completion(int status) {
		this.status = status;
	}
	
	public int getStatus() {
		return status;
	}
	
	public void setStatus(int status) {
		this.status = status;
	}	
};
