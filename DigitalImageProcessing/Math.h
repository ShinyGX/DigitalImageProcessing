#pragma once
#include <cassert>
#include <complex>

namespace ImageUtil
{
	const double PI = 3.1415926;

	namespace Math {
		template<typename T, int Col = 3>
		struct Vector
		{
			T column[Col];

			Vector()
				= default;

			T& operator[](int index)
			{
				assert(index >= 0 && index < Col);
				return column[index];
			}

			Vector<T, Col>& operator*(Vector<T, Col> v)
			{
				for (int i = 0; i < Col; i++)
				{
					column[i] *= v[i];
				}

				return *this;
			}

			void logThis()
			{
				for (T& e : column)
				{
					std::cout << e << " ";
				}
				std::cout << std::endl;
			}
		};

		template<typename T, int Col, int Row>
		struct Matrix
		{
			Vector<T, Row> row[Col];

			Matrix()
				= default;


			explicit Matrix(std::initializer_list<T> args)
			{
				reset(args);
			}

			void reset(std::initializer_list<T> args)
			{
				const int length = args.size();

				int index = -1;
				for (int i = 0; i < Col; i++)
				{
					for (int j = 0; j < Row; j++)
					{
						if (index < length - 1)
							++index;

						row[i][j] = *(args.begin() + index);

					}
				}
			}

			Vector<T, Row>& operator[](int index)
			{
				assert(index < Col && index >= 0);
				return row[index];
			}

			friend std::ostream& operator<<(std::ostream &os, Matrix<T, Col, Row>& m)
			{
				for (int i = 0; i < Col; i++)
				{
					for (int j = 0; j < Row; j++)
					{
						os << m[i][j] << " ";
					}
					os << std::endl;
				}
				os << std::endl;
				return os;
			}

			template<int _Row>
			Matrix<T, Col, _Row> operator*(Matrix<T, Row, _Row>& o)
			{
				Matrix<T, Col, _Row> result;
				for (int i = 0; i < Col; i++)
				{
					for (int j = 0; j < _Row; j++)
					{
						result[i][j] = 0;
						for (int r = 0; r < Row; r++)
						{
							result[i][j] += row[i][r] * o[r][j];
						}
					}
				}
				return result;
			}

			void logThis()
			{
				for (Vector<T, Row>& e : row)
				{
					e.logThis();
				}
				std::cout << std::endl;
			}
		};


		typedef Matrix<int, 3, 3> Matrix3x3i;
		typedef Matrix<double, 3, 3> Matrix3x3d;

		typedef Matrix<int, 3, 1> Matrix3x1i;
		typedef Matrix<double, 3, 1> Matrix3x1d;

	}


}
