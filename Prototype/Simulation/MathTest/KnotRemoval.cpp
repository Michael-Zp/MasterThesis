#include "KnotRemoval.h"


struct Strand
{
	float Knot[32];
	float KnotValues[32];
	float MaxKnotValue;
};


bool floatEqual(float a, float b, float epsilon)
{
	return a - epsilon < b && a + epsilon > b;
}

void KnotRemoval::Test()
{
	Strand strand{ {0, 0, 0, 0, 1, 2, 2, 2, 2}, {0, 0.33, 0.67, 1, 2}, 2 };

	int i = 1;
	int MAX_KNOT_SIZE = 32;

	float factorWhetherPointAtIorIPlus1 = 1.0;

	float myKnotValue = strand.KnotValues[i];
	float otherKnotValue = strand.KnotValues[i + 1];

	//int r = 0;
	//strand.Knot[r++] = 0;
	//strand.Knot[r++] = 0;
	//strand.Knot[r++] = 0;
	//strand.Knot[r++] = 0;
	//strand.Knot[r++] = myKnotValue + factorWhetherPointAtIorIPlus1 * 0.34 * otherKnotValue;
	//strand.Knot[r++] = 2;
	//strand.Knot[r++] = 2;
	//strand.Knot[r++] = 2;
	//strand.Knot[r++] = 2;
	//strand.MaxKnotValue = 2;



	float epsilon = 1e-3;


	// TODO Make without ifs should be easy with steps
	// Start at k = 4 because the first 4 zeros should never be removed
	// The MaxKnotValue (0 0 0 0 1 2 2 2 2 => 2) should also not be removed, but the KnotValue should never be 2
	// because it needs 3 polygons with 4 points and the second knotValue is always the value of third point, thus
	// never the last point in the spline thus never MaxKnotValue
	for (int k = 4, nk = k; nk < MAX_KNOT_SIZE - 3; k++, nk++)
	{
		if (floatEqual(myKnotValue, strand.Knot[k], epsilon))
		{
			nk++;
		}

		if (floatEqual(otherKnotValue, strand.Knot[k], epsilon))
		{
			nk++;
		}

		strand.Knot[k] = strand.Knot[nk];
	}


	//// Strat from zero because loop unrolling
	for (int u = 0; u < MAX_KNOT_SIZE - 2; u++)
	{
		if (u >= i)
		{
			strand.KnotValues[u] = strand.KnotValues[u + 2];
		}
		else
		{
			strand.KnotValues[u] = strand.KnotValues[u];
		}
	}


	float newKnotValue = myKnotValue + factorWhetherPointAtIorIPlus1 * 0.34 * (otherKnotValue - myKnotValue);
}