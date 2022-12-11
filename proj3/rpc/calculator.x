struct param_int {
	int x;
	int y;
};

program CAL_PROG {
	version CAL_VERS {
		int CALCULATE_ADD(param_int) = 1;
		int CALCULATE_SUB(param_int) = 2;
		int CALCULATE_MUL(param_int) = 3;
		int CALCULATE_DIV(param_int) = 4;
		int CALCULATE_POW(param_int) = 5;
	} = 1;
} = 0x31111111;
