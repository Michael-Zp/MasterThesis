using System.Collections.Generic;
using UnityEngine;

public class PageManager : MonoBehaviour
{
    [System.Serializable]
    public struct Step
    {
        public UnityEngine.UI.Image ImageObj;
        public UnityEngine.Sprite ReplacementImage;
        public int MaxWidthReplacementImage;
        public int MaxHeightReplacementImage;
        public UnityEngine.UI.Text TextObj;
        public GameObject GeneralObject;
        public bool DoAdditionalStep;
        public bool NewVisiblity;
    };

    public string Title;
    public string Subtitle;
    public List<Step> Steps;
    public GameObject Page;

    private Step CurrentStep => Steps[CurrentStepIndex];
    private int CurrentStepIndex = 0;

    public bool ShowNext()
    {
        CurrentStepIndex++;
        if(CurrentStepIndex >= Steps.Count)
        {
            return false;
        }

        if(CurrentStep.ImageObj != null)
        {
            CurrentStep.ImageObj.enabled = CurrentStep.NewVisiblity;
            if(CurrentStep.ReplacementImage != null)
            {
                CurrentStep.ImageObj.sprite = CurrentStep.ReplacementImage;
                CurrentStep.ImageObj.SetNativeSize();
                Vector2 scaleFactors = new Vector2(CurrentStep.MaxWidthReplacementImage / CurrentStep.ImageObj.rectTransform.rect.width, CurrentStep.MaxHeightReplacementImage / CurrentStep.ImageObj.rectTransform.rect.height);
                float scaleFactor = Mathf.Min(scaleFactors.x, scaleFactors.y);
                CurrentStep.ImageObj.rectTransform.localScale = new Vector3(scaleFactor, scaleFactor, 1);
            }
        }

        if(CurrentStep.TextObj != null)
        {
            CurrentStep.TextObj.enabled = CurrentStep.NewVisiblity;
        }

        if(CurrentStep.GeneralObject != null)
        {
            CurrentStep.GeneralObject.SetActive(CurrentStep.NewVisiblity);
        }

        if(CurrentStep.DoAdditionalStep)
        {
            ShowNext();
        }

        return true;
    }

    public void Show()
    {
        Page.SetActive(true);
        CurrentStepIndex = -1;
        ShowNext();
    }

    public void Hide()
    {
        Page.SetActive(false);
    }
}
