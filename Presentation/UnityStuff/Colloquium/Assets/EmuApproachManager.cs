using System.Collections.Generic;
using UnityEngine;

public class EmuApproachManager : MonoBehaviour, ICallMeMaybe
{
    public StrandTractrixExample TractrixExample;
    public InputManager InputManager;
    public GameObject PrefabArrow;
    public GameObject PrefabText;
    public GameObject PrefabNewPoint;
    public Canvas CurrentCanvas;
    public UnityEngine.UI.Text AlgoText;

    private List<GameObject> _tempGameObjects = new List<GameObject>();
    private GameObject LatestTempObj => _tempGameObjects[_tempGameObjects.Count - 1];

    private enum States
    {
        GlobalForces,
        CalcNextPositions,
        RandomOrder,
        MovePartikle0,
        MovePartikle1,
        MovePartikle2,
        MovePartikle3,
        MoveTail,
        LocalForces,
        HairstyleForces
    }

    private class State
    {
        private readonly States[] _stateOrder =
        {
            States.GlobalForces,
            States.CalcNextPositions,
            States.RandomOrder,
            States.MovePartikle0,
            States.MovePartikle1,
            States.MovePartikle2,
            States.MovePartikle3,
            States.MoveTail,
            States.LocalForces,
            States.HairstyleForces
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

    private int[] RandomOrder;

    private void Start()
    {
        RandomOrder = new int[TractrixExample.MAX_PARTICLE_COUNT];

        int[] orderPool = new int[TractrixExample.MAX_PARTICLE_COUNT];

        for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
        {
            orderPool[i] = i;
        }

        for(int i = orderPool.Length - 1; i >= 0; i--)
        {
            int nextIdx = Random.Range(0, i + 1);
            RandomOrder[i] = orderPool[nextIdx];
            orderPool[nextIdx] = orderPool[i];
        }
    }

    private List<Vector3> nextCalcedPosis = new List<Vector3>();

    public void CallMeOnRightArrow()
    {
        switch (_currentState.CurrentState)
        {
            case States.GlobalForces:
                AlgoText.text = @"<b>Algorithmus:</b>
    <color='blue'>1. Globale Kräfte pro Partikel berechnen</color>
    2. Nächste Positionen berechnen
    3. Zufällige Reihenfolge bestimmen
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    5. Haar zurück an Wurzel ziehen
    6. Lokale Kräfte pro Partikel berechnen
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for(int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    Vector3 randomVec = new Vector3(Random.Range(-0.3f, 0.3f), Random.Range(-0.3f, 0.3f), 0);
                    if(randomVec.x == 0)
                    {
                        randomVec += new Vector3(0, 0.1f, 0);
                    }
                    Vector3 newPos = TractrixExample.GetPos(i) + randomVec;
                    nextCalcedPosis.Add(newPos);
                    Vector3 arrowPos = TractrixExample.GetPos(i);
                    _tempGameObjects.Add(Instantiate(PrefabArrow, CurrentCanvas.transform));
                    LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(arrowPos);
                    LatestTempObj.transform.localScale *= 0.5f;
                    float dot = Vector3.Dot(randomVec.normalized, Vector3.down);
                    float zRot = 0;
                    if(randomVec.x > 0)
                    {
                        dot = (dot + 1) / 2;
                        zRot = 180 - dot * 180;
                    }
                    else
                    {
                        dot = (dot + 1) / 2;
                        zRot = 180 + dot * 180;
                    }
                    LatestTempObj.transform.rotation = Quaternion.Euler(0, 0, zRot);
                }
                break;
            case States.CalcNextPositions:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    <color='blue'>2. Nächste Positionen berechnen</color>
    3. Zufällige Reihenfolge bestimmen
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    5. Haar zurück an Wurzel ziehen
    6. Lokale Kräfte pro Partikel berechnen
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects.Add(Instantiate(PrefabNewPoint));
                    LatestTempObj.transform.position = nextCalcedPosis[i];
                }
                break;
            case States.RandomOrder:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    2. Nächste Positionen berechnen
    <color='blue'>3. Zufällige Reihenfolge bestimmen</color>
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    5. Haar zurück an Wurzel ziehen
    6. Lokale Kräfte pro Partikel berechnen
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    Vector3 newPos = TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0);
                    _tempGameObjects.Add(Instantiate(PrefabText, CurrentCanvas.transform));
                    LatestTempObj.transform.position = Camera.main.WorldToScreenPoint(newPos);
                    LatestTempObj.transform.GetComponent<UnityEngine.UI.Text>().text = "<b>" + (RandomOrder[i] + 1) + "</b>";
                }
                break;
            case States.MovePartikle0:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    2. Nächste Positionen berechnen
    3. Zufällige Reihenfolge bestimmen
    <color='blue'>4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)</color>
    5. Haar zurück an Wurzel ziehen
    6. Lokale Kräfte pro Partikel berechnen
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    if(RandomOrder[i] == 0)
                    {
                        TractrixExample.MoveParticle(i, nextCalcedPosis[i] - TractrixExample.GetPos(i), 50);
                    }
                }
                for(int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i));
                    _tempGameObjects[i + TractrixExample.MAX_PARTICLE_COUNT * 2].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0));
                }
                break;
            case States.MovePartikle1:
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    if (RandomOrder[i] == 1)
                    {
                        TractrixExample.MoveParticle(i, nextCalcedPosis[i] - TractrixExample.GetPos(i), 50);
                    }
                }
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i));
                    _tempGameObjects[i + TractrixExample.MAX_PARTICLE_COUNT * 2].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0));
                }
                break;
            case States.MovePartikle2:
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    if (RandomOrder[i] == 2)
                    {
                        TractrixExample.MoveParticle(i, nextCalcedPosis[i] - TractrixExample.GetPos(i), 50);
                    }
                }
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i));
                    _tempGameObjects[i + TractrixExample.MAX_PARTICLE_COUNT * 2].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0));
                }
                break;
            case States.MovePartikle3:
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    if (RandomOrder[i] == 3)
                    {
                        TractrixExample.MoveParticle(i, nextCalcedPosis[i] - TractrixExample.GetPos(i), 50);
                    }
                }
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i));
                    _tempGameObjects[i + TractrixExample.MAX_PARTICLE_COUNT * 2].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0));
                }
                
                break;
            case States.MoveTail:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    2. Nächste Positionen berechnen
    3. Zufällige Reihenfolge bestimmen
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    <color='blue'>5. Haar zurück an Wurzel ziehen</color>
    6. Lokale Kräfte pro Partikel berechnen
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    Destroy(_tempGameObjects[_tempGameObjects.Count - 1 - TractrixExample.MAX_PARTICLE_COUNT - i]);
                }
                _tempGameObjects.RemoveRange(TractrixExample.MAX_PARTICLE_COUNT, TractrixExample.MAX_PARTICLE_COUNT);
                TractrixExample.MoveTailToOrigin(50);
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i));
                    _tempGameObjects[i + TractrixExample.MAX_PARTICLE_COUNT * 1].transform.position = Camera.main.WorldToScreenPoint(TractrixExample.GetPos(i) + new Vector3(0.4f, 0.2f, 0));
                }
                break;
            case States.LocalForces:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    2. Nächste Positionen berechnen
    3. Zufällige Reihenfolge bestimmen
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    5. Haar zurück an Wurzel ziehen
    <color='blue'>6. Lokale Kräfte pro Partikel berechnen</color>
    7. Kraft in Richtung der gewünschten
        Frisur berechnen";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.Rotate(new Vector3(0, 0, Random.Range(-20f, 20f) + (Random.Range((int)0, 2) - 1) * 40));
                }
                break;
            case States.HairstyleForces:
                AlgoText.text = @"<b>Algorithmus:</b>
    1. Globale Kräfte pro Partikel berechnen
    2. Nächste Positionen berechnen
    3. Zufällige Reihenfolge bestimmen
    4. Für Partikel p in Reihenfolge
            4.1. Partikel p bewegen (Tractrix)
    5. Haar zurück an Wurzel ziehen
    6. Lokale Kräfte pro Partikel berechnen
    <color='blue'>7. Kraft in Richtung der gewünschten
        Frisur berechnen</color>";
                for (int i = 0; i < TractrixExample.MAX_PARTICLE_COUNT; i++)
                {
                    _tempGameObjects[i].transform.Rotate(new Vector3(0, 0, Random.Range(-70f, 70f) + (Random.Range((int)0, 2) - 1) * 40));
                }
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
        InputManager.EventBitches.Remove(this);
        for(int i = 0; i < _tempGameObjects.Count; i++)
        {
            Destroy(_tempGameObjects[0]);
        }
    }
}
