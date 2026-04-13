#!/usr/bin/env python3
"""
Plot EventBuilder performance metrics from event_performance.csv

Usage:
    python plot_performance.py [csvfile] [output]

    csvfile: Path to event_performance.csv (default: event_performance.csv)
    output:  Output file path (default: event_performance.png)
"""

import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

def plot_performance(csv_file="event_performance.csv", output_file="event_performance.png"):
    """
    Read performance CSV and create comprehensive performance plots
    """
    try:
        # Read CSV file
        df = pd.read_csv(csv_file)
        print(f"Loaded {len(df)} events from {csv_file}")
        print(f"Columns: {list(df.columns)}")
        
        # Create figure with subplots
        fig = plt.figure(figsize=(18, 12))
        gs = gridspec.GridSpec(3, 3, figure=fig, hspace=0.35, wspace=0.35)
        
        # Plot 1: Event build time vs event ID
        ax1 = fig.add_subplot(gs[0, :2])
        ax1.plot(df['event_id'], df['event_build_time_ms'], 'b-', linewidth=1, alpha=0.7)
        ax1.set_xlabel('Event ID')
        ax1.set_ylabel('Event Build Time (ms)')
        ax1.set_title('Event Build Time vs Event ID')
        ax1.grid(True, alpha=0.3)
        
        # Plot 2: Event build time histogram
        ax1b = fig.add_subplot(gs[0, 2])
        ax1b.hist(df['event_build_time_ms'], bins=30, color='blue', alpha=0.7, edgecolor='black')
        ax1b.set_xlabel('Build Time (ms)')
        ax1b.set_ylabel('Frequency')
        ax1b.set_title('Build Time Distribution')
        ax1b.grid(True, alpha=0.3, axis='y')
        
        # Plot 3: Cumulative vs Windowed Events/sec
        ax2 = fig.add_subplot(gs[1, 0])
        ax2.plot(df['event_id'], df['cum_events_per_sec'], 'g-', linewidth=2, alpha=0.7, label='Cumulative')
        ax2.plot(df['event_id'], df['window_events_per_sec'], 'orange', linewidth=1, alpha=0.5, label='Window (100-event)')
        ax2.set_xlabel('Event ID')
        ax2.set_ylabel('Events/sec')
        ax2.set_title('Throughput: Events/sec (Cumulative vs Windowed)')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
        
        # Plot 4: Cumulative vs Windowed MB/sec
        ax3 = fig.add_subplot(gs[1, 1])
        ax3.plot(df['event_id'], df['cum_mb_per_sec'], 'r-', linewidth=2, alpha=0.7, label='Cumulative')
        ax3.plot(df['event_id'], df['window_mb_per_sec'], 'purple', linewidth=1, alpha=0.5, label='Window (100-event)')
        ax3.set_xlabel('Event ID')
        ax3.set_ylabel('MB/sec')
        ax3.set_title('Throughput: MB/sec (Cumulative vs Windowed)')
        ax3.legend()
        ax3.grid(True, alpha=0.3)
        
        # Plot 5: Only windowed events/sec (zoomed)
        ax4 = fig.add_subplot(gs[1, 2])
        ax4.plot(df[df['window_events_per_sec'] > 0]['event_id'], 
                df[df['window_events_per_sec'] > 0]['window_events_per_sec'], 
                'o-', linewidth=1, markersize=3, alpha=0.7, color='orange')
        ax4.set_xlabel('Event ID')
        ax4.set_ylabel('Events/sec')
        ax4.set_title('Windowed Throughput (100-event window)')
        ax4.grid(True, alpha=0.3)
        
        # Plot 6: Total events built (cumulative)
        ax5 = fig.add_subplot(gs[2, 0])
        ax5.plot(df['event_id'], df['total_events'], 'purple', linewidth=2, alpha=0.7)
        ax5.set_xlabel('Event ID')
        ax5.set_ylabel('Total Events Built')
        ax5.set_title('Cumulative Events Built vs Event ID')
        ax5.grid(True, alpha=0.3)
        
        # Plot 7: Total bytes read (cumulative)
        ax6 = fig.add_subplot(gs[2, 1])
        ax6.plot(df['event_id'], df['total_bytes_mb'], 'orange', linewidth=2, alpha=0.7)
        ax6.set_xlabel('Event ID')
        ax6.set_ylabel('Total Bytes (MB)')
        ax6.set_title('Cumulative Data Read (MB) vs Event ID')
        ax6.grid(True, alpha=0.3)
        
        # Plot 8: Summary statistics table
        ax7 = fig.add_subplot(gs[2, 2])
        ax7.axis('off')
        
        # Calculate statistics - include both cumulative and windowed
        window_data = df[df['window_events_per_sec'] > 0]
        stats = [
            ['CUMULATIVE METRICS', ''],
            ['Total Events', f"{df['total_events'].iloc[-1]:.0f}"],
            ['Total Data (MB)', f"{df['total_bytes_mb'].iloc[-1]:.2f}"],
            ['Avg Events/sec', f"{df['cum_events_per_sec'].mean():.2f}"],
            ['Avg MB/sec', f"{df['cum_mb_per_sec'].mean():.2f}"],
            ['', ''],
            ['WINDOWED METRICS (100-event)', ''],
            ['Avg Events/sec', f"{df['window_events_per_sec'].mean():.2f}"],
            ['Avg MB/sec', f"{df['window_mb_per_sec'].mean():.2f}"],
            ['Max Events/sec', f"{df['window_events_per_sec'].max():.2f}"],
            ['Max MB/sec', f"{df['window_mb_per_sec'].max():.2f}"],
            ['', ''],
            ['EVENT BUILD TIME', ''],
            ['Avg (ms)', f"{df['event_build_time_ms'].mean():.2f}"],
            ['Min (ms)', f"{df['event_build_time_ms'].min():.2f}"],
            ['Max (ms)', f"{df['event_build_time_ms'].max():.2f}"],
        ]
        
        table = ax7.table(cellText=stats, 
                          colLabels=['Metric', 'Value'],
                          cellLoc='left',
                          loc='center',
                          colWidths=[0.55, 0.45])
        table.auto_set_font_size(False)
        table.set_fontsize(9)
        table.scale(1, 1.5)
        
        # Style header row
        for i in range(2):
            table[(0, i)].set_facecolor('#40466e')
            table[(0, i)].set_text_props(weight='bold', color='white')
        
        # Style section headers and alternate row colors
        section_headers = [1, 7, 13]
        for i in range(1, len(stats) + 1):
            for j in range(2):
                if i in section_headers:
                    table[(i, j)].set_facecolor('#d0d0d0')
                    table[(i, j)].set_text_props(weight='bold')
                elif i == 5 or i == 11:  # Empty rows
                    table[(i, j)].set_facecolor('white')
                elif i % 2 == 0:
                    table[(i, j)].set_facecolor('#f0f0f0')
                else:
                    table[(i, j)].set_facecolor('white')
        
        # Main title
        fig.suptitle(f'EventBuilder Performance Metrics ({len(df)} events)', 
                     fontsize=16, fontweight='bold')
        
        # Save figure
        plt.savefig(output_file, dpi=150, bbox_inches='tight')
        print(f"\nPlot saved to {output_file}")
        
        # Print summary statistics to console
        print("\n" + "="*50)
        print("PERFORMANCE SUMMARY")
        print("="*50)
        for metric, value in stats:
            print(f"{metric:.<30} {value:>15}")
        print("="*50)
        
        return fig
        
    except FileNotFoundError:
        print(f"Error: CSV file '{csv_file}' not found")
        print("Make sure you've run EventBuilder and event_performance.csv was created")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    csv_file = sys.argv[1] if len(sys.argv) > 1 else "event_performance.csv"
    output_file = sys.argv[2] if len(sys.argv) > 2 else "event_performance.png"
    
    fig = plot_performance(csv_file, output_file)
    
    # Optionally display the plot
    try:
        plt.show()
    except:
        pass
