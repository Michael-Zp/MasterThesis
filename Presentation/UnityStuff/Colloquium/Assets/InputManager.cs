using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class InputManager : MonoBehaviour
{
    public UnityEngine.UI.Text TitleObj;
    public UnityEngine.UI.Text SubTitleObj;
    public List<PageManager> Pages;
    public List<ICallMeMaybe> EventBitches = new List<ICallMeMaybe>();

    private PageManager ActivePage => Pages[ActivePageIndex];
    private int ActivePageIndex;

    void Start()
    {
        ActivePageIndex = 0;
        ShowActivePage();
    }

    private void ShowActivePage()
    {
        ActivePage.Show();
        TitleObj.text = ActivePage.Title;
        SubTitleObj.text = ActivePage.Subtitle;
    }

    void Update()
    {
        if(Input.GetKeyDown(KeyCode.RightArrow) || Input.GetMouseButtonDown(2))
        {
            if(!ActivePage.ShowNext())
            {
                if(ActivePageIndex < Pages.Count)
                {
                    ActivePage.Hide();
                    ActivePageIndex++;
                    ShowActivePage();
                }
            }

            foreach(var bitch in EventBitches)
            {
                bitch.CallMeOnRightArrow();
            }
        }
    }
}
