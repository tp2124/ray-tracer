#pragma once

#include <cassert>

typedef enum {ZERO, NO_INIT, IDENTITY, CATMULROM_SPLINE_BASIS, CUBIC_HERMITE_SPLINE_BASIS} MatrixArg;

template <int row = 4, int col = 4>
class Matrix
{
public:

	Matrix()
	{
		Matrix::Matrix(ZERO);
	}

	~Matrix()
	{
		/*for(int y = 0; y < m_col; y++)
		{
			delete [] &m_data[y];
		}
		delete [] &m_data;*/
	}

	Matrix(MatrixArg arg)
	{
		m_col = col;
		m_row = row;
		switch(arg)
		{
			case NO_INIT:
				break;
			case ZERO:
				loadZeroMatrix();
				break;
			case IDENTITY:
				loadIdentityMatrix();
				break;
			case CATMULROM_SPLINE_BASIS:
				loadCatmulRomSplineBasis();
				break;
			case CUBIC_HERMITE_SPLINE_BASIS:
				loadCubicHermiteSplineBasis();
				break;
		}
	}

	void loadParameterVector(float param)
	{
		assert(m_col == 4 && m_row == 1);

		float val = param;

		m_data[0][2] = val;
		val*=param;
		m_data[0][1] = val;
		val*=param;
		m_data[0][0] = val;
		m_data[0][3] = 1;
	}

	void loadZeroMatrix()
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				m_data[y][x] = 0;
			}
		}
	}

	void loadIdentityMatrix()
	{
		loadZeroMatrix();
		int x, y;
		x = y = 0;
		while(x < m_col && y < m_row)
		{
			m_data[y][x] = 1;
			x++;
			y = x;
		}
	}

	void loadCatmulRomSplineBasis(float s = 0.5f)
	{
		assert(row == 4 && col == 4);

		//-s	2-s		s-2		s
		//2s	s-3		3-2s	-s
		// -s	0		s		0
		// 0	1		0		0	
		

		m_data[0][0] = -s;
		m_data[0][1] = 2-s;
		m_data[0][2] = s-2;
		m_data[0][3] = s;

		m_data[1][0] = 2*s;
		m_data[1][1] = s-3;
		m_data[1][2] = 3-2*s;
		m_data[1][3] = -s;

		m_data[2][0] = -s;
		m_data[2][1] = 0;
		m_data[2][2] = s;
		m_data[2][3] = 0;

		m_data[3][0] = 0;
		m_data[3][1] = 1;
		m_data[3][2] = 0;
		m_data[3][3] = 0;
	}

	void loadCubicHermiteSplineBasis()
	{
		assert(row == 4 && col == 4);

		m_data[0][0] = 2;
		m_data[0][1] = -2;
		m_data[0][2] = 1;
		m_data[0][3] = 1;

		m_data[1][0] = -3;
		m_data[1][1] = 3;
		m_data[1][2] = -2;
		m_data[1][3] = -1;

		m_data[2][0] = 0;
		m_data[2][1] = 0;
		m_data[2][2] = 1;
		m_data[2][3] = 0;

		m_data[3][0] = 1;
		m_data[3][1] = 0;
		m_data[3][2] = 0;
		m_data[3][3] = 0;
	}
		
	Matrix<row, col>(Matrix<row, col> const &other)
	{
		m_col = other.m_col; //equals this->m_col too
		m_row = other.m_row;

		setValues(other.m_data);
	}

	bool operator==(const Matrix<row, col> &other)
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				if(m_data[y][x] != other.m_data[y][x])
					return false;
			}
		}
		return true;
	}
	bool operator!=(const Matrix<row, col> &other)
	{
		return !(*this==other);
	}

	Matrix<row, col>& operator=(Matrix<row, col> &other)
	{
		m_col = other.m_col; //equals this->m_col too
		m_row = other.m_row;

		setValues(other.m_data);

		return *this;
	}

	Matrix<row, col>& operator+=(const Matrix<row, col> &other)
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{

				m_data[y][x] += other.m_data[y][x];
			}
		}

		return *this;
	}
	Matrix<row, col>& operator-=(const Matrix<row, col> &other)
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{

				m_data[y][x] -= other.m_data[y][x];
			}
		}

		return *this;
	}

	const Matrix<row, col> operator+(const Matrix<row, col> &other)
	{
		Matrix<row, col> result = *this;
		result+=other;
		return result;
	}
	const Matrix<row, col> operator-(const Matrix<row, col> &other)
	{
		Matrix<row, col> result = *this;
		result-=other;
		return result;
	}

	Matrix<row, col>& operator*=(float num)
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				m_data[y][x] *=num;
			}
		}

		return *this;
	}
	Matrix<row, col>& operator/=(float num)
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				m_data[y][x] /=num;
			}
		}

		return *this;
	}
	const Matrix<row, col> operator*(float num)
	{
		Matrix<row, col> result = *this;
		result*=num;
		return result;
	}
	const Matrix<row, col> operator/(float num)
	{
		Matrix<row, col> result = *this;
		result/=num;
		return result;
	}

	template<int opRow>
	Matrix<row, opRow> operator*(Matrix<col, opRow> &other)
	{
		Matrix<row, opRow> result = Matrix<row, opRow>(NO_INIT);

		for(int y = 0; y < result.m_row; y++)
		{
			for(int x = 0; x < result.m_col; x++)
			{
				result.m_data[y][x] = 0;
				for(int k=0;k<m_col;++k)
				{
					result.m_data[y][x] += m_data[y][k] * other.m_data[k][x];
				}
			}
		}

		return result;
	}

	void setRow(int rowNum, const float(&rowVals)[col])
	{
		if(rowNum >=0 && rowNum < m_row)
		{
			for(int i = 0; i < col; i++)
			{
				m_data[rowNum][i] = rowVals[i];
			}
		}
	}

	const float(&getRow(int rowNum) const) [col]
	{
		assert(rowNum >=0 && rowNum < m_row);
		{
			return m_data[rowNum];
		}
	}

	void setValues(const float(&values)[row][col])
	{
		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				m_data[y][x] = values[y][x];
			}
		}
	}

	float** getValues() const
	{
		return m_data;
	}

	const float(&getValuesAsOneDimensionalArray() const) [col * row]
	{
		float array[col * row];

		for(int y = 0; y < m_row; y++)
		{
			for(int x = 0; x < m_col; x++)
			{
				array[y*col + x] = m_data[y][x];
			}
		}
		return array;
	}

	float m_data[row][col];

	int m_col;
	int m_row;

private:

};
