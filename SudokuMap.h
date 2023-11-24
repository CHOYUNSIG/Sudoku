#pragma once

#include <vector>





class SudokuMap {
private:
	int m_lpMap[9][9] = { 0, };
	int m_lpMemo[9][9] = { 0, };
	int m_nBlank = 0;

public:
	SudokuMap(int del);
	SudokuMap(int pre_map[9][9], int pre_memo[9][9]);
	int GetValue(int row, int col) const;
	int GetMemo(int row, int col) const;
	bool Editable(int row, int col) const;
	void SetValue(int value, int row, int col);
	bool ToggleMemo(int value, int row, int col);
	bool Contradict() const;
	bool Contradict(int row, int col) const;
	bool Done() const;
};
