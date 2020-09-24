using UnityEngine;
using BansheeGz.BGSpline.Curve;

public class BGCurveInitialize : MonoBehaviour
{
    public GameObject BGCurveObj;
    public StrandTractrixExample Tractrix;

    private void OnEnable()
    {
        var bgCurve = BGCurveObj.GetComponent<BGCurve>();
        bgCurve.Clear();

        var points = Tractrix.GetPoints();

        bool chosenDir = true;
        for (int i = 0; i < points.Length; i++)
        {
            Vector3 controlFirst = new Vector3(0, 0, 0);
            if (i - 1 >= 0)
            {
                Vector3 segment = points[i - 1] - points[i];
                controlFirst = segment / 3;
                Vector3 orthogonal;
                if (chosenDir)
                {
                    orthogonal = new Vector3(segment.y, -segment.x, 0);
                }
                else
                {
                    orthogonal = new Vector3(-segment.y, segment.x, 0);
                }
                orthogonal.Normalize();
                controlFirst += orthogonal * segment.magnitude / 3;
            }

            Vector3 controlSecond = new Vector3(0, 0, 0);
            if (i + 1 < points.Length)
            {
                Vector3 segment = points[i + 1] - points[i];
                controlSecond = segment / 3;
                Vector3 orthogonal;
                if (chosenDir)
                {
                    orthogonal = new Vector3(segment.y, -segment.x, 0);
                }
                else
                {
                    orthogonal = new Vector3(-segment.y, segment.x, 0);
                }
                orthogonal.Normalize();
                controlSecond += orthogonal * segment.magnitude / 3;
            }

            bgCurve.AddPoint(new BGCurvePoint(bgCurve, points[i], BGCurvePoint.ControlTypeEnum.BezierIndependant, controlFirst, controlSecond));
            chosenDir = !chosenDir;
        }
        BGCurveObj.GetComponent<LineRenderer>().startWidth = 0;
        BGCurveObj.GetComponent<LineRenderer>().endWidth = 0;

        BGCurveObj.SetActive(true);
        Destroy(Tractrix.gameObject.GetComponent<LineRenderer>());
    }

    private int waitFrames = 10;
    private bool once = true;

    private void Update()
    {
        if (once)
        {
            waitFrames--;
            if (waitFrames == 0)
            {
                BGCurveObj.GetComponent<LineRenderer>().startWidth = 0.15f;
                BGCurveObj.GetComponent<LineRenderer>().endWidth = 0.15f;
                once = false;
            }
        }
    }
}
