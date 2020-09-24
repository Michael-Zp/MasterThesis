using System.Collections.Generic;
using UnityEngine;

public class SplitApproachManager : MonoBehaviour, ICallMeMaybe
{
    public StrandTractrixExample TractrixExample;
    public InputManager InputManager;
    public GameObject PrefabArrow;
    public Canvas CurrentCanvas;
    public UnityEngine.UI.Text AlgoText;

    private List<GameObject> _tempGameObjects = new List<GameObject>();
    private GameObject LatestTempObj => _tempGameObjects[_tempGameObjects.Count - 1];

    private enum States
    {
        HeadVelocity,
        MoveHead,
        MoveTail,
        ReduceSpeed,
        Mix
    }

    private class State
    {
        private readonly States[] _stateOrder =
        {
            States.HeadVelocity,
            States.MoveHead,
            States.MoveTail,
            States.ReduceSpeed,
            States.Mix
        };
        private int _stateIndex = 0;

        public States CurrentState { get; private set; }

        public State()
        {
            CurrentState = _stateOrder[_stateIndex];
        }

        public static State operator ++(State state)
        {
            state._stateIndex = Mathf.Min(state._stateIndex + 1, state._stateOrder.Length - 1);
            state.CurrentState = state._stateOrder[state._stateIndex];
            return state;
        }
    }

    private State _currentState = new State();

    public void CallMeOnRightArrow()
    {
        switch (_currentState.CurrentState)
        {
            case States.HeadVelocity:
                AlgoText.text = @"<b>Algorithmus:</b>
	<color=blue>1. Globale Kräfte auf Kopf anwenden</color>
	2. Kopf bewegen (Tractrix)
	3. Haar zurück an Wurzel ziehen
	4. Kopfgeschwindigkeit reduzieren
	5. Simulation und Frisur mixen";
                Vector3 arrowPos = TractrixExample.GetHeadPos() + new Vector3(0, -0.5f, 0);
                _tempGameObjects.Add(Instantiate(PrefabArrow, CurrentCanvas.transform));
                LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(arrowPos);
                
                break;
            case States.MoveHead:
                AlgoText.text = @"<b>Algorithmus:</b>
	1. Globale Kräfte auf Kopf anwenden
	<color=blue>2. Kopf bewegen (Tractrix)</color>
	3. Haar zurück an Wurzel ziehen
	4. Kopfgeschwindigkeit reduzieren
	5. Simulation und Frisur mixen";
                TractrixExample.MoveHead(new Vector3(0, -2, 0), 50);

                arrowPos = TractrixExample.GetHeadPos() + new Vector3(0, -0.5f, 0);
                LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(arrowPos);
                break;
            case States.MoveTail:
                AlgoText.text = @"<b>Algorithmus:</b>
	1. Globale Kräfte auf Kopf anwenden
	2. Kopf bewegen (Tractrix)
	<color=blue>3. Haar zurück an Wurzel ziehen</color>
	4. Kopfgeschwindigkeit reduzieren
	5. Simulation und Frisur mixen";
                TractrixExample.MoveTailToOrigin(50);

                arrowPos = TractrixExample.GetHeadPos() + new Vector3(0, -0.5f, 0);
                LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(arrowPos);
                break;
            case States.ReduceSpeed:
                AlgoText.text = @"<b>Algorithmus:</b>
	1. Globale Kräfte auf Kopf anwenden
	2. Kopf bewegen (Tractrix)
	3. Haar zurück an Wurzel ziehen
	<color=blue>4. Kopfgeschwindigkeit reduzieren</color>
	5. Simulation und Frisur mixen";
                Vector3 scale = LatestTempObj.transform.localScale;
                LatestTempObj.transform.localScale = new Vector3(scale.x * 0.7f, scale.y * .5f, scale.z);
                arrowPos = TractrixExample.GetHeadPos() + new Vector3(0, -0.35f, 0);
                LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(arrowPos);
                break;
            case States.Mix:
                AlgoText.text = @"<b>Algorithmus:</b>
	1. Globale Kräfte auf Kopf anwenden
	2. Kopf bewegen (Tractrix)
	3. Haar zurück an Wurzel ziehen
	4. Kopfgeschwindigkeit reduzieren
	<color=blue>5. Simulation und Frisur mixen</color>";
                Destroy(LatestTempObj);
                break;
        }

        _currentState++;
    }

    private void OnEnable()
    {
        InputManager.EventBitches.Add(this);
    }

    private void OnDisable()
    {
        AlgoText.text = @"<b>Algorithmus:</b>
	1. Globale Kräfte auf Kopf anwenden
	2. Kopf bewegen (Tractrix)
	3. Haar zurück an Wurzel ziehen
	4. Kopfgeschwindigkeit reduzieren
	<color=blue>5. Simulation und Frisur mixen</color>";
        InputManager.EventBitches.Remove(this);
        Destroy(LatestTempObj);
    }
}
