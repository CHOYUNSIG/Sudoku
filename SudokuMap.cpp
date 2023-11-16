#include "pch.h"
#include "SudokuMap.h"

#include <cstdlib>
#include <algorithm>
#include <utility>
#include <vector>





bool generate_all(int map[9][9], int pos) {
	if (pos == 9 * 9) return true;
	int now = rand() % 9 + 1;
	for (int i = 0; i < 9; i++) {
		bool passed = true;
		for (int j = 0; j < 9; j++)
			if (map[j][pos % 9] == now || map[pos / 9][j] == now || map[(pos / 9) / 3 * 3 + j / 3][(pos % 9) / 3 * 3 + j % 3] == now)
				passed = false;
		if (passed) {
			map[pos / 9][pos % 9] = now;
			if (generate_all(map, pos + 1)) return true;
			map[pos / 9][pos % 9] = 0;
		}
		now = now % 9 + 1;
	}
	return false;
}

SudokuMap::SudokuMap(int del)
{
	for (int i = 0; i < 9 * 9; i++)
		map[i / 9][i % 9] = 0;
	generate_all(map, 0);
	for (int i = 0; i < 9 * 9; i++)
		map[i / 9][i % 9] += 10;

	while (blank < del) {
		int r = rand() % 9;
		int c = rand() % 9;
		if (map[r][c] > 0) {
			map[r][c] = 0;
			blank++;
		}
	}
}

SudokuMap::SudokuMap(int pre_map[9][9])
{
	for (int i = 0; i < 9 * 9; i++) {
		map[i / 9][i % 9] = pre_map[i / 9][i % 9];
		if (map[i / 9][i % 9] == 0)
			blank++;
	}
}

int SudokuMap::GetValue(int row, int col)
{
	return map[row][col] % 10;
}

bool SudokuMap::Editable(int row, int col)
{
	return map[row][col] < 10;
}

void SudokuMap::SetValue(int value, int row, int col)
{
	if (map[row][col] < 10){
		if (map[row][col] > 0 && value == 0)
			blank++;
		else if (map[row][col] == 0 && value > 0)
			blank--;
		map[row][col] = value;
	}
}

bool SudokuMap::Contradict()
{
	for (int i = 0; i < 9; i++) {
		int row = 0;
		int col = 0;
		int box = 0;
		for (int j = 0; j < 9; j++) {
			int row_now = 1 << (map[i][j] % 10);
			int col_now = 1 << (map[j][i] % 10);
			int box_now = 1 << (map[i / 3 * 3 + j / 3][i / 3 * 3 + j % 3] % 10);
			if ((row_now > 1 && row & row_now) || (col_now > 1 && col & col_now) || (box_now > 1 && box & box_now))
				return true;
			else {
				row |= row_now;
				col |= col_now;
				box |= box_now;
			}
		}
	}
	return false;
}

bool SudokuMap::Contradict(int row, int col)
{
	if (map[row][col] == 0)
		return false;
	for (int i = 0; i < 9; i++)
		if ((i != row && map[i][col] % 10 == map[row][col] % 10) ||
			(i != col && map[row][i] % 10 == map[row][col] % 10) ||
			(row / 3 * 3 + i / 3 != row && col != col / 3 * 3 + i % 3 && map[row / 3 * 3 + i / 3][col / 3 * 3 + i % 3] % 10 == map[row][col] % 10))
			return true;
	return false;
}

bool SudokuMap::Done()
{
	if (blank == 0 && !Contradict())
		return true;
	else
		return false;
}
