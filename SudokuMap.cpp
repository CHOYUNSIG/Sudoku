#include "pch.h"
#include "SudokuMap.h"

#include <cstdlib>
#include <algorithm>
#include <utility>
#include <vector>





inline static bool contradict(int map[9][9], int row, int col)
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

inline static bool generate_all(int map[9][9], int pos)
{
	if (pos == 81) return true;
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

inline static int bitsum(int n)
{
	int result = 0;
	for (int i = 0; i < 9; i++)
		if (n & (1 << i))
			result++;
	return result;
}

inline static int bit2num(int bit)
{
	for (int start = rand() % 9, i = start, now = start; now < start + 9; now++, i = now % 9)
		if (bit & (1 << i))
			return i + 1;
	return 0;
}

inline static bool subset(int bit[9][9])
{
	bool changed = false;
	for (int pos = 0; pos < 9; pos++) {
		int cord[3][9][2] = { 0, };
		for (int i = 0; i < 9; i++) {
			cord[0][i][0] = pos;
			cord[0][i][1] = i;
			cord[1][i][0] = i;
			cord[1][i][1] = pos;
			cord[2][i][0] = pos / 3 * 3 + i / 3;
			cord[2][i][1] = pos % 3 * 3 + i % 3;
		}
		for (int set = 1; set <= 8; set++) {
			std::vector<bool> com;
			for (int i = 0; i < 9 - set; i++)
				com.push_back(false);
			for (int i = 0; i < set; i++)
				com.push_back(true);
			do {
				int b[3] = { 0, 0, 0 };
				for (int i = 0; i < 9; i++)
					for (int j = 0; j < 3; j++)
						if (com[i])
							b[j] |= bit[cord[j][i][0]][cord[j][i][1]];
				for (int i = 0; i < 3; i++)
					if (bitsum(b[i]) == set) {
						for (int j = 0; j < 9; j++)
							if (!com[j]) {
								int pre = bit[cord[i][j][0]][cord[i][j][1]];
								bit[cord[i][j][0]][cord[i][j][1]] &= ~b[i];
								if (bit[cord[i][j][0]][cord[i][j][1]] != pre)
									changed = true;
							}
					}
			} while (std::next_permutation(com.begin(), com.end()));
		}
	}
	return changed;
}

inline static bool intersection(int bit[9][9])
{
	bool changed = false;
	for (int pos = 0; pos < 9; pos++) {
		int core[6][3][2] = { 0, };
		int group[6][2][6][2] = { 0, };
		for (int r = 0; r < 3; r++) {
			for (int p = 0; p < 3; p++) {
				core[r][p][0] = pos / 3 * 3 + r;
				core[r][p][1] = pos % 3 * 3 + p;
				core[r + 3][p][0] = pos / 3 * 3 + p;
				core[r + 3][p][1] = pos % 3 * 3 + r;
			}
			for (int p = 0; p < 3; p++) {
				for (int q = 0; q < 2; q++) {
					group[r][0][p + 3 * q][0] = core[r][p][0];
					group[r][0][p + 3 * q][1] = (pos % 3 * 3 + 3 + p + 3 * q) % 9;
					group[r + 3][0][p + 3 * q][0] = (pos / 3 * 3 + 3 + p + 3 * q) % 9;
					group[r + 3][0][p + 3 * q][1] = core[r + 3][p][1];
				}
			}
			for (int p = 0; p < 3; p++) {
				for (int q = 0; q < 2; q++) {
					group[r][1][p + 3 * q][0] = pos / 3 * 3 + (r + 1 + q) % 3;
					group[r][1][p + 3 * q][1] = core[r][p][1];
					group[r + 3][1][p + 3 * q][0] = core[r + 3][p][0];
					group[r + 3][1][p + 3 * q][1] = pos % 3 * 3 + (r + 1 + q) % 3;
				}
			}
		}
		for (int i = 0; i < 6; i++) {
			int bcore = 0;
			int bgroup[2] = { 0, 0 };
			for (int j = 0; j < 3; j++)
				bcore |= bit[core[i][j][0]][core[i][j][1]];
			for (int j = 0; j < 6; j++)
				for (int k = 0; k < 2; k++)
					bgroup[k] |= bit[group[i][k][j][0]][group[i][k][j][1]];
			for (int j = 0; j < 2; j++) {
				int mask = bcore & ~bgroup[j];
				for (int k = 0; k < 6; k++) {
					int pre = bit[group[i][(j + 1) % 2][k][0]][group[i][(j + 1) % 2][k][1]];
					bit[group[i][(j + 1) % 2][k][0]][group[i][(j + 1) % 2][k][1]] &= ~mask;
					if (bit[group[i][(j + 1) % 2][k][0]][group[i][(j + 1) % 2][k][1]] != pre)
						changed = true;
				}
			}
		}
	}
	return changed;
}

inline static bool generate_unique(int map[9][9], int bit[9][9])
{
	int row = -1, col = -1;
	for (int start = rand() % 81, pos = start, now = start; now < start + 81; now++, pos = now % 81) {
		int a = bitsum(bit[pos / 9][pos % 9]);
		if (a == 0)
			return false;
		else if (a > 1) {
			row = pos / 9;
			col = pos % 9;
		}
	}

	if (row == -1 && col == -1)
		return true;

	int pre_bit[9][9] = { 0, };
	for (int i = 0; i < 81; i++)
		pre_bit[i / 9][i % 9] = bit[i / 9][i % 9];

	for (int start = rand() % 9, i = start, now = start; now < start + 9; now++, i = now % 9) {
		if (bit[row][col] & (1 << i)) {
			map[row][col] = i + 1;
			bit[row][col] = (1 << i);
			while (subset(bit) || intersection(bit))
				continue;
			if (generate_unique(map, bit))
				return true;
			map[row][col] = 0;
			for (int i = 0; i < 81; i++)
				bit[i / 9][i % 9] = pre_bit[i / 9][i % 9];
		}
	}

	return false;
}

SudokuMap::SudokuMap(int del)
{
	int bit[9][9] = { 0, };
	for (int i = 0; i < 81; i++) {
		m_lpMap[i / 9][i % 9] = 0;
		bit[i / 9][i % 9] = 0b111111111;
	}

	generate_unique(m_lpMap, bit);

	for (int i = 0; i < 81; i++)
		if (m_lpMap[i / 9][i % 9] == 0)
			m_nBlank++;
	for (; m_nBlank >= del; m_nBlank--) {
		int row, col;
		do {
			row = rand() % 9;
			col = rand() % 9;
		} while (m_lpMap[row][col] > 0);
		m_lpMap[row][col] = bit2num(bit[row][col]);
	}

	for (int i = 0; i < 81; i++)
		if (m_lpMap[i / 9][i % 9] > 0)
			m_lpMap[i / 9][i % 9] += 10;
}

SudokuMap::SudokuMap(int pre_map[9][9])
{
	for (int i = 0; i < 9 * 9; i++) {
		m_lpMap[i / 9][i % 9] = pre_map[i / 9][i % 9];
		if (m_lpMap[i / 9][i % 9] == 0)
			m_nBlank++;
	}
}

int SudokuMap::GetValue(int row, int col) const
{
	return m_lpMap[row][col] % 10;
}

bool SudokuMap::Editable(int row, int col) const
{
	return m_lpMap[row][col] < 10;
}

void SudokuMap::SetValue(int value, int row, int col)
{
	if (m_lpMap[row][col] < 10){
		if (m_lpMap[row][col] > 0 && value == 0)
			m_nBlank++;
		else if (m_lpMap[row][col] == 0 && value > 0)
			m_nBlank--;
		m_lpMap[row][col] = value;
	}
}

bool SudokuMap::Contradict() const
{
	for (int i = 0; i < 9; i++) {
		int row = 0;
		int col = 0;
		int box = 0;
		for (int j = 0; j < 9; j++) {
			int row_now = 1 << (m_lpMap[i][j] % 10);
			int col_now = 1 << (m_lpMap[j][i] % 10);
			int box_now = 1 << (m_lpMap[i / 3 * 3 + j / 3][i / 3 * 3 + j % 3] % 10);
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

bool SudokuMap::Contradict(int row, int col) const
{
	if (m_lpMap[row][col] == 0)
		return false;
	for (int i = 0; i < 9; i++)
		if ((i != row && m_lpMap[i][col] % 10 == m_lpMap[row][col] % 10) ||
			(i != col && m_lpMap[row][i] % 10 == m_lpMap[row][col] % 10) ||
			(row / 3 * 3 + i / 3 != row && col != col / 3 * 3 + i % 3 && m_lpMap[row / 3 * 3 + i / 3][col / 3 * 3 + i % 3] % 10 == m_lpMap[row][col] % 10))
			return true;
	return false;
}

bool SudokuMap::Done() const
{
	if (m_nBlank == 0 && !Contradict())
		return true;
	else
		return false;
}
