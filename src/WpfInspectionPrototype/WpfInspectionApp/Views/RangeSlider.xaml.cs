using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace WpfInspectionApp.Views;

// 2-thumb range slider. Min/Max 를 한 트랙에 두 thumb 로 표현.
// 참조 pemtoFramework 의 UI_Common.CtrlEnableChange_Range_2D 동작을 RangeMode 에 매칭:
//   RangeMode 0 (Inside)  : 두 thumb 활성
//   RangeMode 1 (Outside) : 두 thumb 활성
//   RangeMode 2 (Upper)   : Min thumb 비활성, Min=Minimum 고정
//   RangeMode 3 (Lower)   : Max thumb 비활성, Max=Maximum 고정
//
// MVVM 친화 — Minimum / Maximum / MinValue / MaxValue / RangeMode 의 DependencyProperty
// 만 노출. ValueChanged routed event 가 기존 AlignControl_Changed 패턴과 호환.
public partial class RangeSlider : UserControl
{
    public static readonly DependencyProperty MinimumProperty =
        DependencyProperty.Register(
            nameof(Minimum),
            typeof(double),
            typeof(RangeSlider),
            new PropertyMetadata(0.0, OnRangeBoundsChanged));

    public static readonly DependencyProperty MaximumProperty =
        DependencyProperty.Register(
            nameof(Maximum),
            typeof(double),
            typeof(RangeSlider),
            new PropertyMetadata(255.0, OnRangeBoundsChanged));

    public static readonly DependencyProperty MinValueProperty =
        DependencyProperty.Register(
            nameof(MinValue),
            typeof(double),
            typeof(RangeSlider),
            new FrameworkPropertyMetadata(
                0.0,
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                OnValuesChanged));

    public static readonly DependencyProperty MaxValueProperty =
        DependencyProperty.Register(
            nameof(MaxValue),
            typeof(double),
            typeof(RangeSlider),
            new FrameworkPropertyMetadata(
                255.0,
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                OnValuesChanged));

    public static readonly DependencyProperty RangeModeProperty =
        DependencyProperty.Register(
            nameof(RangeMode),
            typeof(int),
            typeof(RangeSlider),
            new FrameworkPropertyMetadata(
                0,
                FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
                OnRangeModeChanged));

    public static readonly RoutedEvent ValueChangedEvent =
        EventManager.RegisterRoutedEvent(
            nameof(ValueChanged),
            RoutingStrategy.Bubble,
            typeof(RoutedEventHandler),
            typeof(RangeSlider));

    public double Minimum { get => (double)GetValue(MinimumProperty); set => SetValue(MinimumProperty, value); }
    public double Maximum { get => (double)GetValue(MaximumProperty); set => SetValue(MaximumProperty, value); }
    public double MinValue { get => (double)GetValue(MinValueProperty); set => SetValue(MinValueProperty, value); }
    public double MaxValue { get => (double)GetValue(MaxValueProperty); set => SetValue(MaxValueProperty, value); }
    public int RangeMode { get => (int)GetValue(RangeModeProperty); set => SetValue(RangeModeProperty, value); }

    public event RoutedEventHandler ValueChanged
    {
        add => AddHandler(ValueChangedEvent, value);
        remove => RemoveHandler(ValueChangedEvent, value);
    }

    private bool _suppressValueChangedEvent;

    public RangeSlider()
    {
        InitializeComponent();
        SizeChanged += (_, _) => UpdateLayoutPositions();
        Loaded += (_, _) =>
        {
            ApplyRangeModeEnableState();
            UpdateLayoutPositions();
        };
    }

    private void MinThumb_DragDelta(object sender, DragDeltaEventArgs e)
    {
        if (!MinThumb.IsEnabled)
        {
            return;
        }

        var trackWidth = ThumbCanvas.ActualWidth;
        if (trackWidth <= 0)
        {
            return;
        }

        var deltaValue = e.HorizontalChange * (Maximum - Minimum) / trackWidth;
        var newMin = MinValue + deltaValue;
        // Min 은 [Minimum, MaxValue] 범위 내에서만 이동 (Max 보다 위로 못 감).
        newMin = Math.Max(Minimum, Math.Min(MaxValue, newMin));
        SetCurrentValue(MinValueProperty, newMin);
    }

    private void MaxThumb_DragDelta(object sender, DragDeltaEventArgs e)
    {
        if (!MaxThumb.IsEnabled)
        {
            return;
        }

        var trackWidth = ThumbCanvas.ActualWidth;
        if (trackWidth <= 0)
        {
            return;
        }

        var deltaValue = e.HorizontalChange * (Maximum - Minimum) / trackWidth;
        var newMax = MaxValue + deltaValue;
        // Max 는 [MinValue, Maximum] 범위 내에서만 이동.
        newMax = Math.Min(Maximum, Math.Max(MinValue, newMax));
        SetCurrentValue(MaxValueProperty, newMax);
    }

    // RangeMode 변경 시 참조 동작 매칭 — Upper/Lower 모드일 때 비활성 thumb 의 값을 끝점으로 고정.
    private void ApplyRangeModeEnableState()
    {
        try
        {
            _suppressValueChangedEvent = true;
            switch (RangeMode)
            {
                case 2: // Upper: Min thumb 잠금, Min = Minimum
                    MinThumb.IsEnabled = false;
                    MaxThumb.IsEnabled = true;
                    if (Math.Abs(MinValue - Minimum) > 0.5)
                    {
                        SetCurrentValue(MinValueProperty, Minimum);
                    }
                    break;
                case 3: // Lower: Max thumb 잠금, Max = Maximum
                    MinThumb.IsEnabled = true;
                    MaxThumb.IsEnabled = false;
                    if (Math.Abs(MaxValue - Maximum) > 0.5)
                    {
                        SetCurrentValue(MaxValueProperty, Maximum);
                    }
                    break;
                default: // Inside (0) / Outside (1): 둘 다 활성
                    MinThumb.IsEnabled = true;
                    MaxThumb.IsEnabled = true;
                    break;
            }
        }
        finally
        {
            _suppressValueChangedEvent = false;
        }
    }

    private void UpdateLayoutPositions()
    {
        var trackWidth = ThumbCanvas.ActualWidth;
        if (trackWidth <= 0)
        {
            return;
        }

        var minPx = ValueToPixel(MinValue, trackWidth);
        var maxPx = ValueToPixel(MaxValue, trackWidth);
        Canvas.SetLeft(MinThumb, Math.Max(0, minPx - MinThumb.Width / 2));
        Canvas.SetLeft(MaxThumb, Math.Max(0, maxPx - MaxThumb.Width / 2));
        ActiveBar.Margin = new Thickness(minPx, 0, 0, 0);
        ActiveBar.Width = Math.Max(0, maxPx - minPx);
    }

    private double ValueToPixel(double value, double trackWidth)
    {
        var span = Math.Max(0.0001, Maximum - Minimum);
        return trackWidth * (value - Minimum) / span;
    }

    private static void OnValuesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var rs = (RangeSlider)d;
        rs.UpdateLayoutPositions();
        if (!rs._suppressValueChangedEvent)
        {
            rs.RaiseEvent(new RoutedEventArgs(ValueChangedEvent, rs));
        }
    }

    private static void OnRangeBoundsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        ((RangeSlider)d).UpdateLayoutPositions();
    }

    private static void OnRangeModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
        var rs = (RangeSlider)d;
        rs.ApplyRangeModeEnableState();
        rs.UpdateLayoutPositions();
    }
}
