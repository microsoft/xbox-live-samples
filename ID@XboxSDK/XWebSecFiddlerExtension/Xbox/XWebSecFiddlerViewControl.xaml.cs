using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace XWebSecFiddlerExtension
{
    /// <summary>
    /// Interaction logic for XWebSecFiddlerViewControl.xaml
    /// </summary>
    public partial class XWebSecFiddlerViewControl : UserControl
    {
      //  public delegate void RefreshEventHandler();
       // public event RefreshEventHandler RefreshChanged;

     //   public delegate void SelectedEventHandler(string domain);
     //   public event SelectedEventHandler SelectedChanged;

        private Object updateLock;
        private int[] uiIndex;

        List<int> selectedFilterRows;
        List<int> selectedHttpRows;
        List<int> selectedHttpsRows;

        public XWebSecFiddlerViewControl()
        {
            InitializeComponent();
            updateLock = new Object();
            uiIndex = new int[3] { -1, -1, -1 };
            selectedFilterRows = new List<int>();
            selectedHttpRows = new List<int>();
            selectedHttpsRows = new List<int>();

            var version = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            VersionLabel.Content = String.Format("v.{0}.{1}.{2}", version.Major, version.Minor, String.Format(".{0}", version.Build));
        }

        internal void UpdateUI(Types.XWebSecData domainData)
        {
            Dispatcher.InvokeAsync(new Action(() =>
            {
                lock (updateLock)
                {
                    // update status tab
                    if (domainData.httpDomainCount + domainData.httpsDomainCount + domainData.filteredDomainCount > 0)
                    {
                        if (domainData.httpsDomainList.Count > 0)
                        {
                            StatusText.Content = "Pass";
                            StatusText.Foreground = new SolidColorBrush(Colors.Green);
                            HttpsDomains.Foreground = new SolidColorBrush(Colors.Green);
                        }

                        if (domainData.httpDomainList.Count > 0)
                        {
                            StatusText.Content = "Failure";
                            StatusText.Foreground = new SolidColorBrush(Colors.Red);
                            HttpDomains.Foreground = new SolidColorBrush(Colors.Red);
                        }
                    }
                    else
                    {
                        StatusText.Content = "n/a";
                        StatusText.Foreground = new SolidColorBrush(Colors.Gray);
                        HttpsDomains.Content = "n/a";
                        HttpsDomains.Foreground = new SolidColorBrush(Colors.Gray);
                        HttpDomains.Content = "n/a";
                        HttpDomains.Foreground = new SolidColorBrush(Colors.Gray);
                    }

                    HttpsDomains.Content = domainData.httpsDomainCount;
                    HttpDomains.Content = domainData.httpDomainCount;
                    ExcludedDomains.Content = domainData.filteredDomainCount;

                    //Update data for lists
                    FilteredDataGrid.ItemsSource = from row in domainData.filteredDomainList.ToList()
                        select new
                        {
                            filterCount = row.Value,
                            filterHttps = row.Key.httpsFormatted,
                            filterDomainName = row.Key.domainName,
                            filterWS = row.Key.webSocketFormatted
                        };

                    HttpsDataGrid.ItemsSource = from row in domainData.httpsDomainList.ToList()
                        select new
                        {
                            count = row.Value,
                            domainName = row.Key.domainName,
                            ws = row.Key.webSocketFormatted
                        };

                    HttpDataGrid.ItemsSource = from row in domainData.httpDomainList.ToList()
                        select new
                        {
                            count = row.Value,
                            domainName = row.Key.domainName,
                            ws = row.Key.webSocketFormatted
                        };

                    //Restore cached selection
                    if (DomainFilterTab.IsSelected)
                    {
                        foreach (int i in selectedFilterRows)
                        {
                            FilteredDataGrid.SelectedItems.Add(FilteredDataGrid.Items.GetItemAt(i));
                        }

                        FilteredDataGrid.Focus();
                    }

                    if (DomainTab.IsSelected)
                    {
                        if (selectedHttpRows.Count > 0)
                        {
                            foreach (int i in selectedHttpRows)
                            {
                                HttpDataGrid.SelectedItems.Add(HttpDataGrid.Items.GetItemAt(i));
                            }
                            HttpDataGrid.Focus();
                        }

                        if (selectedHttpsRows.Count > 0)
                        {
                            foreach (int i in selectedHttpsRows)
                            {
                                HttpsDataGrid.SelectedItems.Add(HttpsDataGrid.Items.GetItemAt(i));
                            }
                            HttpsDataGrid.Focus();
                        }
                    }
                }
            }));
        }

        private void UpdateLink_Click(object sender, RoutedEventArgs e)
        {
            // TODO: check for update online
        }

        public void EnableLoadingOverlay()
        {
            LoadBackground.Visibility = Visibility.Visible;
            LoadProcessingLabel.Visibility = Visibility.Visible;
            LoadProgressBar.Visibility = Visibility.Visible;
        }

        public void DisableLoadingOverlay()
        {
            LoadBackground.Visibility = Visibility.Hidden;
            LoadProcessingLabel.Visibility = Visibility.Hidden;
            LoadProgressBar.Visibility = Visibility.Hidden;
        }

        //Cache selection indexes on selection change
        private void OnFilterListSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            selectedFilterRows.Clear();

            foreach (object o in FilteredDataGrid.SelectedItems)
            {
                selectedFilterRows.Add(FilteredDataGrid.Items.IndexOf(o));
            }
        }

        private void OnHttpsListSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            selectedHttpsRows.Clear();

            foreach (object o in HttpsDataGrid.SelectedItems)
            {
                selectedHttpRows.Add(HttpDataGrid.Items.IndexOf(o));
            }
        }

        private void OnHttpListSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            selectedHttpRows.Clear();

            foreach (object o in HttpDataGrid.SelectedItems)
            {
                selectedHttpsRows.Add(FilteredDataGrid.Items.IndexOf(o));
            }
        }
    }
}
