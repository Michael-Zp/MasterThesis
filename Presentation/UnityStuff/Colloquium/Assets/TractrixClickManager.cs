using UnityEngine;

public class TractrixClickManager : MonoBehaviour
{
    public StrandTractrixExample TractrixExample;

    private Vector3 _offsetToCenterOfDraggedObj;
    private GameObject _dragObj;
    private bool _isDragging = false;
    private Vector3 _oldMousePos;

    void Update()
    {
        if(Input.GetMouseButtonDown(0))
        {
            Vector3 mousePos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            Vector2 mousePos2D = new Vector2(mousePos.x, mousePos.y);

            RaycastHit2D hit = Physics2D.Raycast(mousePos2D, Vector2.zero);
            if(hit.collider != null)
            {
                if(hit.collider.gameObject.tag == "TractrixPoint")
                {
                    _isDragging = true;
                    _dragObj = hit.collider.gameObject;
                    _offsetToCenterOfDraggedObj = mousePos - hit.collider.gameObject.transform.position;
                    _offsetToCenterOfDraggedObj.z = 0;
                    _oldMousePos = mousePos;
                }
            }
        } 
        else if(Input.GetMouseButtonUp(0))
        {
            if(_isDragging)
            {
                _isDragging = false;
            }
        }


        if(_isDragging)
        {  
            Vector3 mousePos = Camera.main.ScreenToWorldPoint(Input.mousePosition);
            if(_oldMousePos != mousePos)
            {
                TractrixExample.UpdateParticle(_dragObj, new Vector3(mousePos.x, mousePos.y, _dragObj.transform.position.z) - _offsetToCenterOfDraggedObj);
            }
            _oldMousePos = mousePos;
        }
    }
}
