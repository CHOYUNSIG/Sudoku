#pragma once




class SudokuMap {
private:
	int m_lpMap[9][9];
	int m_nBlank = 0;

public:
	SudokuMap(int del);
	SudokuMap(int pre_map[9][9]);
	int GetValue(int row, int col);
	bool Editable(int row, int col);
	void SetValue(int value, int row, int col);
	bool Contradict();
	bool Contradict(int row, int col);
	bool Done();
};
