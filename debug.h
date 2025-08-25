class mClass {
	void(mClass::*FuncPtr)();
	void mFunc() {
		//My code
	}
	void n() {
		FuncPtr = &mFunc;
		(this->*FuncPtr)();
	}
};
