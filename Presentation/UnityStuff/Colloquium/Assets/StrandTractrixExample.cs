using System.Collections.Generic;
using UnityEngine;

public class StrandTractrixExample : MonoBehaviour
{
    public GameObject StartPoint;
    public List<Vector3> Directions;
    public LineRenderer LineRenderer;

    internal void MoveParticle(int idx, Vector3 movement, int iterations)
    {
        GameObject head = _particlePoints[idx];
        for (int i = 0; i < iterations; i++)
        {
            UpdateParticle(head, head.transform.position + movement / iterations);
        }
    }

    internal void MoveHead(Vector3 movement, int iterations)
    {
        GameObject head = _particlePoints[_particlePoints.Count - 1];
        for(int i = 0; i < iterations; i++)
        {
            UpdateParticle(head, head.transform.position + movement / iterations);
        }
    }

    internal void MoveTailToOrigin(int iterations)
    {
        GameObject tail = _particlePoints[0];
        Vector3 movement = StartPosition - tail.transform.position;
        for (int i = 0; i < iterations; i++)
        {
            UpdateParticle(tail, tail.transform.position + movement / iterations);
        }
    }

    public GameObject PrefabParticlePoints;
    public float SegmentLength;
    
    private Vector3 StartPosition;
    private float _segmentLengthLastFrame;
    private List<GameObject> _particlePoints = new List<GameObject>();

    public Vector3 GetPos(int idx)
    {
        return _particlePoints[idx].transform.position;
    }

    public Vector3 GetHeadPos()
    {
        return GetPos(_particlePoints.Count - 1);
    }

    

    public Vector3[] GetPoints()
    {
        Vector3[] vec = new Vector3[_particlePoints.Count];
        for(int i = 0; i < _particlePoints.Count; i++)
        {
            vec[i] = _particlePoints[i].transform.position;
        }
        return vec;
    }

    // Start is called before the first frame update
    void Start()
    {
        StartPosition = StartPoint.transform.position;
        Destroy(StartPoint);
        InitializeLines();
    }

    float sechInv(float z)
    {
        //http://mathworld.wolfram.com/InverseHyperbolicSecant.html
        //return log(sqrt(1 / z - 1) * sqrt(1 / z + 1) + 1 / z);
        //But this seems to be for a complex number
        //thus
        //https://en.wikipedia.org/wiki/Inverse_hyperbolic_functions
        return Mathf.Log((1 + Mathf.Sqrt(1 - (z * z))) / z);
    }

    float sech(float z)
    {
        //http://mathworld.wolfram.com/HyperbolicSecant.html
        //this seems to be identical with https://en.wikipedia.org/wiki/Hyperbolic_function
        return 1 / (float)System.Math.Cosh(z);
    }

    struct TractrixStepReturn
    {
        public Vector3 NewTailPos;
        public Vector3 NewHeadPos;
    };


    struct Matrix3x3
    {
        //_mRowColumn
        public float _m11;
        public float _m12;
        public float _m13;
        public float _m21;
        public float _m22;
        public float _m23;
        public float _m31;
        public float _m32;
        public float _m33;

        public Matrix3x3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33)
        {
            _m11 = m11;
            _m12 = m12;
            _m13 = m13;
            _m21 = m21;
            _m22 = m22;
            _m23 = m23;
            _m31 = m31;
            _m32 = m32;
            _m33 = m33;
        }

        public static Vector3 operator *(Vector3 v, Matrix3x3 m)
        {
            return new Vector3(m._m11 * v.x + m._m12 * v.y + m._m13 * v.z, m._m21 * v.x + m._m22 * v.y + m._m23 * v.z, m._m31 * v.x + m._m32 * v.y + m._m33 * v.z);
        }

        public Matrix3x3 Transposed()
        {
            return new Matrix3x3(_m11, _m12, _m13, _m21, _m22, _m23, _m31, _m32, _m33);
        }
    }

    public int MAX_PARTICLE_COUNT = 5;
    private int Case1Count = 0;
    private int Case2Count = 0;
    private int Case3Count = 0;


    TractrixStepReturn TractrixStep(Vector3 tailPos, Vector3 headPos, Vector3 desiredHeadPos)
    {
        TractrixStepReturn ret;
        ret.NewHeadPos = headPos;
        ret.NewTailPos = tailPos;

        Vector3 S = desiredHeadPos - headPos;

        float lengthS = Vector3.SqrMagnitude(S);

        float L = Vector3.Magnitude(headPos - tailPos);

        Vector3 T = tailPos - headPos;


        //TODO See if this still a bug
        //See if they are linear independent, if cross == (0, 0, 0) they are not
        //If they are linear dependent, just add some stuff on T to get a correct coordinate system 
        //Otherwise Vector3.Cross(S, T) = Vector3(0, 0, 0) and this is not good.
        //If they just pull it straight the tail should follow exactly the same
        Vector3 tempCross = Vector3.Cross(S, T);
        Vector3 crossST = tempCross.normalized;
        if (crossST.normalized == new Vector3(0, 0, 0) || S.normalized == new Vector3(0, 0, 0))
        {
            Case1Count++;
            ret.NewHeadPos = desiredHeadPos;
            ret.NewTailPos = desiredHeadPos - (headPos - tailPos);
            //strands[0].Color = float4(1, 0, 0, 1);
        }
        else
        {
            Vector3 Xr = Vector3.Normalize(S);

            Vector3 Zr = Vector3.Normalize(Vector3.Cross(S, T));

            Vector3 Yr = Vector3.Cross(Zr, Xr);
            

            Matrix3x3 R = new Matrix3x3(Xr.x, Xr.y, Xr.z, Yr.x, Yr.y, Yr.z, Zr.x, Zr.y, Zr.z);
            R = R.Transposed();

            float y = Vector3.Dot(Yr, T);


            float p_p = L * sechInv(y / L) + lengthS;
            float p_n = L * sechInv(y / L) - lengthS;

            //Prevent stuff from jumping to harsh, because sechInv -> infinity if x -> 0
            p_p = Mathf.Clamp(p_p, -100, 100);

            float xr_p = +lengthS - L * (float)System.Math.Tanh(p_p / L);
            float xr_n = -lengthS - L * (float)System.Math.Tanh(p_p / L);

            float xr_np_p = +lengthS - L * (float)System.Math.Tanh(p_n / L);
            float xr_np_n = -lengthS - L * (float)System.Math.Tanh(p_n / L);

            float yr_p = L * sech(p_p / L);
            float yr_n = L * sech(p_n / L);


            Vector3 tempPos = new Vector3(xr_p, yr_p, 0);




            if (Vector3.SqrMagnitude(tailPos - desiredHeadPos) > Vector3.SqrMagnitude(tailPos - headPos))  // Replacable by 'dot(Vector3.Normalize(T), Vector3.Normalize(S)) < 0' [for performance, because length of S and T might be used elsewhere too]
            {
                tempPos = new Vector3(xr_p, yr_p, 0);
            }
            else
            {
                // Adjustment by me. Looks more realistic, if the motion of the head is in the direction of X
                tempPos = new Vector3(-xr_np_n, yr_n, 0);
            }


            Vector3 rotatedTempPos = tempPos * R;
            float temp = rotatedTempPos.x;
            rotatedTempPos.x = rotatedTempPos.y;
            rotatedTempPos.y = temp;
            Vector3 newTailPos = headPos + new Vector3(rotatedTempPos.x, rotatedTempPos.y, rotatedTempPos.z);

            //if (Vector3.SqrMagnitude(newTailPos - tailPos) < Vector3.SqrMagnitude(desiredHeadPos - headPos))
            if(false)
            {
                //Does not work in Unity, but approx is good enough for presentation
                Case2Count++;
                // I don´t have infinite precision and sechInv -> infinity if x -> 0 so yeah... should prevent these ehm irregularities (basically everything just fucks up)
                // Additionaly it is mathematically correct, because in a tractrix the movement of the tail has to be smaller than the movement of the head (thats the whole point of this thing)
                rotatedTempPos = tempPos * R;
                ret.NewTailPos = headPos + new Vector3(rotatedTempPos.x, rotatedTempPos.y, rotatedTempPos.z);
                ret.NewHeadPos = desiredHeadPos;
                //strands[0].Color = float4(0, 1, 0, 1);

            }
            else
            {
                Case3Count++;
                //If the calculation fails, just set the head to the desiredHeadPos and 
                //pull the tail along the control polygon. This will prevent weird cases when in which
                //the tail will follow the exact same movement as the head (like a z shape falling straight down instead of flexing)
                //strands[0].Color = float4(0, 0, 1, 1);
                ret.NewHeadPos = desiredHeadPos;
                ret.NewTailPos = tailPos + (headPos - tailPos).normalized * (desiredHeadPos - headPos).magnitude;
                ret.NewTailPos = ret.NewHeadPos + Vector3.Normalize(ret.NewTailPos - ret.NewHeadPos) * Mathf.Sqrt(Vector3.Dot(headPos - tailPos, headPos - tailPos));
            }
        }


        return ret;
    }

    private void OnDestroy()
    {
        Debug.Log("Case1: " + Case1Count);
        Debug.Log("Case2: " + Case2Count);
        Debug.Log("Case3: " + Case3Count);
    }


    void RecursiveTractrixForward(int startIndex, Vector3 Xp, out Vector3[] newParticlePositions)
    {
        //See sreenivasan2010.pdf
        //https://doi.org/10.1016/j.mechmachtheory.2009.10.005
        //Chapter 3.1 and 4


        /*
        // S x T --> Crossproduct; |S| --> Length of S; ^T --> Transpose
        (0) //Added from eq. 12// L^2 = (x - xe)^2 + (y - ye)^2 + (z - ze)^2
        (1) Define the vector S = Xp - Xh where Xh is the current location of the head and Xp is the destination point of the head.
        (2) Define the vector T = X - Xh where X = (x; y; z)^T is the tail of the link lying on the tractrix.
        (3) Define the new reference coordinate system frg with the X-axis along S. Hence ^Xr = S / |S|.
        (4) Define the Z-axis as ^Zr = S x T / |S x T|.
        (5) Define rotation matrix [R] = [ ^Xr; ^Zr x ^Xr; ^Zr ].
        (6) The Y-coordinate of the tail (lying on the tractrix) is given by y = dot(^Yr, T) and the parameter p can be obtained as
        p = L * sech^-1(y / L) +- |S|.
        (7) From p, we can obtain the X and Y-coordinate of the point on the tractrix in the reference coordinate system as
        xr = +- |S| - L * tanh(p / L).
        yr = L * sech(p / l).
        (8) Once xr and yr are known, the point on the tractrix (x; y; z)^T in the global fixed coordinate system {0} is given by
        (x; y; z)^T = Xh + [R]*(xr; yr; 0)^T
        */

        newParticlePositions = new Vector3[MAX_PARTICLE_COUNT];

        for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
        {
            newParticlePositions[i] = new Vector3(0, 0, 0);
        }

        if ( /*forwards && */startIndex + 1 < _particlePoints.Count)
        {
            int headParticleIdx = startIndex;
            int tailParticleIdx = headParticleIdx + 1;

            Vector3 Xh = _particlePoints[headParticleIdx].transform.position;
            Vector3 Xt = _particlePoints[tailParticleIdx].transform.position;

            headParticleIdx++;
            tailParticleIdx++;

            TractrixStepReturn tractrixResult;

            for (; tailParticleIdx < _particlePoints.Count; headParticleIdx++, tailParticleIdx++)
            {
                tractrixResult = TractrixStep(Xt, Xh, Xp);

                Xh = _particlePoints[headParticleIdx].transform.position;
                Xt = _particlePoints[tailParticleIdx].transform.position;
                Xp = tractrixResult.NewTailPos;

                newParticlePositions[tailParticleIdx - 1] = tractrixResult.NewTailPos;
            }

            tractrixResult = TractrixStep(Xt, Xh, Xp);

            newParticlePositions[tailParticleIdx - 1] = tractrixResult.NewTailPos;
        }
    }

    void RecursiveTractrixBackward(int startIndex, Vector3 Xp, out Vector3[] newParticlePositions)
    {
        //See sreenivasan2010.pdf
        //https://doi.org/10.1016/j.mechmachtheory.2009.10.005
        //Chapter 3.1 and 4


        /*
        // S x T --> Crossproduct; |S| --> Length of S; ^T --> Transpose
        (0) //Added from eq. 12// L^2 = (x - xe)^2 + (y - ye)^2 + (z - ze)^2
        (1) Define the vector S = Xp - Xh where Xh is the current location of the head and Xp is the destination point of the head.
        (2) Define the vector T = X - Xh where X = (x; y; z)^T is the tail of the link lying on the tractrix.
        (3) Define the new reference coordinate system frg with the X-axis along S. Hence ^Xr = S / |S|.
        (4) Define the Z-axis as ^Zr = S x T / |S x T|.
        (5) Define rotation matrix [R] = [ ^Xr; ^Zr x ^Xr; ^Zr ].
        (6) The Y-coordinate of the tail (lying on the tractrix) is given by y = dot(^Yr, T) and the parameter p can be obtained as
        p = L * sech^-1(y / L) +- |S|.
        (7) From p, we can obtain the X and Y-coordinate of the point on the tractrix in the reference coordinate system as
        xr = +- |S| - L * tanh(p / L).
        yr = L * sech(p / l).
        (8) Once xr and yr are known, the point on the tractrix (x; y; z)^T in the global fixed coordinate system {0} is given by
        (x; y; z)^T = Xh + [R]*(xr; yr; 0)^T
        */

        newParticlePositions = new Vector3[MAX_PARTICLE_COUNT];

        for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
        {
            newParticlePositions[i] = new Vector3(0, 0, 0);
        }

        if ( /*!forwards && */startIndex - 1 >= 0)
        {
            int headParticleIdx = startIndex;
            int tailParticleIdx = headParticleIdx - 1;

            Vector3 Xh = _particlePoints[headParticleIdx].transform.position;
            Vector3 Xt = _particlePoints[tailParticleIdx].transform.position;

            headParticleIdx--;
            tailParticleIdx--;

            TractrixStepReturn tractrixResult;

            for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
            {
                tractrixResult = TractrixStep(Xt, Xh, Xp);

                Xh = _particlePoints[Mathf.Max(headParticleIdx - i, 0)].transform.position;
                Xt = _particlePoints[Mathf.Max(tailParticleIdx - i, 0)].transform.position;
                Xp = tractrixResult.NewTailPos;

                if(tailParticleIdx - i + 1 >= 0 && tailParticleIdx - i + 1 < newParticlePositions.Length)
                {
                    newParticlePositions[Mathf.Max(tailParticleIdx - i + 1, 0)] = tractrixResult.NewTailPos;
                }
            }
        }
    }


    private void MoveForward(int startIndex, Vector3 newHeadPos)
    {
        //Vector3 moveVec = newHeadPos - _particlePoints[startIndex].transform.position;

        //for(int i = startIndex + 1; i < _particlePoints.Count; i++)
        //{
        //    _particlePoints[i].transform.position += moveVec;
        //}

        RecursiveTractrixForward(startIndex, newHeadPos, out Vector3[] newPositions);

        for (int i = startIndex + 1; i < _particlePoints.Count; i++)
        {
            _particlePoints[i].transform.position = newPositions[i];
        }
    }

    private void MoveBackward(int startIndex, Vector3 newHeadPos)
    {
        //Vector3 moveVec = newHeadPos - _particlePoints[startIndex].transform.position;

        //for (int i = startIndex - 1; i >= 0; i--)
        //{
        //    _particlePoints[i].transform.position += moveVec;
        //}
        
        RecursiveTractrixBackward(startIndex, newHeadPos, out Vector3[] newPositions);

        for (int i = startIndex - 1; i >= 0; i--)
        {
            _particlePoints[i].transform.position = newPositions[i];
        }
    }

    public void UpdateParticle(GameObject obj, Vector3 newPos)
    {
        int startIndex = 0;
        for(int i = 0; i < _particlePoints.Count; i++)
        {
            if(_particlePoints[i] == obj)
            {
                startIndex = i;
                break;
            }
        }

        MoveForward(startIndex, newPos);
        MoveBackward(startIndex, newPos);
        obj.transform.position = newPos;

        UpdateLines();
    }

    private void UpdateLines()
    {
        if(LineRenderer != null)
        {
            for (int i = 0; i < _particlePoints.Count; i++)
            {
                LineRenderer.SetPosition(i, _particlePoints[i].transform.position);
            }
        }
    }

    private void InitializeLines()
    {
        Vector3 currentPos = StartPosition;
        List<Vector3> positions = new List<Vector3>();
        
        _particlePoints.Add(Instantiate(PrefabParticlePoints, transform));
        _particlePoints[0].transform.position = currentPos;

        for (int i = 0; i < Directions.Count; i++)
        {
            positions.Add(currentPos);
            currentPos += Vector3.Normalize(Directions[i]) * SegmentLength;

            _particlePoints.Add(Instantiate(PrefabParticlePoints, transform));
            _particlePoints[i + 1].transform.position = currentPos;
        }
        positions.Add(currentPos);

        LineRenderer.positionCount = positions.Count;
        LineRenderer.SetPositions(positions.ToArray());
    }

    // Update is called once per frame
    void Update()
    {
        if(_segmentLengthLastFrame != SegmentLength)
        {
            UpdateLines();
        }

        _segmentLengthLastFrame = SegmentLength;
    }
}
