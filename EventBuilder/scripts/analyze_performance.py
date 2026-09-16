#!/usr/bin/env python3
"""
Analysis script for EventBuilder performance metrics

Creates detailed analysis reports from event_performance.csv
"""

import sys

import numpy as np
import pandas as pd


def analyze_performance(csv_file="event_performance.csv"):
    """
    Read performance CSV and print detailed analysis
    """
    try:
        # Read CSV file
        df = pd.read_csv(csv_file)
        print(f"\n{'=' * 70}")
        print("EVENTBUILDER PERFORMANCE ANALYSIS")
        print(f"{'=' * 70}")
        print(f"Total events processed: {len(df)}")

        # Time-based metrics
        print(f"\n{'EVENT BUILD TIME':-^70}")
        print(f"  Mean:              {df['event_build_time_ms'].mean():>10.3f} ms")
        print(f"  Median:            {df['event_build_time_ms'].median():>10.3f} ms")
        print(f"  Std Dev:           {df['event_build_time_ms'].std():>10.3f} ms")
        print(f"  Min:               {df['event_build_time_ms'].min():>10.3f} ms")
        print(f"  Max:               {df['event_build_time_ms'].max():>10.3f} ms")
        print("  Percentiles:")
        for p in [25, 50, 75, 90, 95, 99]:
            val = np.percentile(df["event_build_time_ms"], p)
            print(f"    {p:>2}th:            {val:>10.3f} ms")

        # Throughput metrics (events/sec) - CUMULATIVE
        c_eps = df["cum_events_per_sec"]
        print(f"\n{'EVENTS/SECOND (CUMULATIVE AVERAGE)':-^70}")
        print(f"  Mean:              {c_eps.mean():>10.2f} events/sec")
        print(f"  Median:            {c_eps.median():>10.2f} events/sec")
        print(f"  Std Dev:           {c_eps.std():>10.2f} events/sec")
        print(f"  Min:               {c_eps.min():>10.2f} events/sec")
        print(f"  Max:               {c_eps.max():>10.2f} events/sec")
        print("  Percentiles:")
        for p in [25, 50, 75, 90, 95, 99]:
            val = np.percentile(c_eps, p)
            print(f"    {p:>2}th:            {val:>10.2f} events/sec")

        # Data throughput metrics (MB/sec) - CUMULATIVE
        print(f"\n{'DATA THROUGHPUT (CUMULATIVE AVERAGE)':-^70}")
        print(f"  Mean:              {df['cum_mb_per_sec'].mean():>10.3f} MB/sec")
        print(f"  Median:            {df['cum_mb_per_sec'].median():>10.3f} MB/sec")
        print(f"  Std Dev:           {df['cum_mb_per_sec'].std():>10.3f} MB/sec")
        print(f"  Min:               {df['cum_mb_per_sec'].min():>10.3f} MB/sec")
        print(f"  Max:               {df['cum_mb_per_sec'].max():>10.3f} MB/sec")
        print("  Percentiles:")
        for p in [25, 50, 75, 90, 95, 99]:
            val = np.percentile(df["cum_mb_per_sec"], p)
            print(f"    {p:>2}th:            {val:>10.3f} MB/sec")

        # Windowed throughput metrics (events/sec) - for real-time monitoring
        window_data = df[
            df["window_events_per_sec"] > 0
        ]  # Only include windows with data
        if len(window_data) > 0:
            w_eps = window_data["window_events_per_sec"]
            print(f"\n{'EVENTS/SECOND (100-EVENT WINDOWED AVERAGE)':-^70}")
            print(f"  Mean:              {w_eps.mean():>10.2f} events/sec")
            print(f"  Median:            {w_eps.median():>10.2f} events/sec")
            print(f"  Std Dev:           {w_eps.std():>10.2f} events/sec")
            print(f"  Min:               {w_eps.min():>10.2f} events/sec")
            print(f"  Max:               {w_eps.max():>10.2f} events/sec")
            print("  Percentiles:")
            for p in [25, 50, 75, 90, 95, 99]:
                val = np.percentile(w_eps, p)
                print(f"    {p:>2}th:            {val:>10.2f} events/sec")

            # Data throughput metrics (MB/sec) - WINDOWED
            w_mbps = window_data["window_mb_per_sec"]
            print(f"\n{'DATA THROUGHPUT (100-EVENT WINDOWED AVERAGE)':-^70}")
            print(f"  Mean:              {w_mbps.mean():>10.3f} MB/sec")
            print(f"  Median:            {w_mbps.median():>10.3f} MB/sec")
            print(f"  Std Dev:           {w_mbps.std():>10.3f} MB/sec")
            print(f"  Min:               {w_mbps.min():>10.3f} MB/sec")
            print(f"  Max:               {w_mbps.max():>10.3f} MB/sec")
            print("  Percentiles:")
            for p in [25, 50, 75, 90, 95, 99]:
                val = np.percentile(w_mbps, p)
                print(f"    {p:>2}th:            {val:>10.3f} MB/sec")

        # Final statistics
        final_row = df.iloc[-1]
        print(f"\n{'FINAL STATISTICS':-^70}")
        print(f"  Total events built:        {int(final_row['total_events']):>10d}")
        print(f"  Total data processed:      {final_row['total_bytes_mb']:>10.2f} MB")
        eps = final_row["cum_events_per_sec"]
        mbps = final_row["cum_mb_per_sec"]
        print(f"  Final events/sec (cumulative):    {eps:>10.2f} events/sec")
        print(f"  Final MB/sec (cumulative):        {mbps:>10.3f} MB/sec")
        if final_row["window_events_per_sec"] > 0:
            eps = final_row["window_events_per_sec"]
            mbps = final_row["window_mb_per_sec"]
            print(f"  Final events/sec (windowed):      {eps:>10.2f} events/sec")
            print(f"  Final MB/sec (windowed):          {mbps:>10.3f} MB/sec")

        # Performance trends
        print(f"\n{'PERFORMANCE TRENDS':-^70}")

        # Calculate rolling averages for cumulative metrics
        window = min(100, len(df) // 10)
        if window > 1:
            rolling_events = df["cum_events_per_sec"].rolling(window=window).mean()
            rolling_mb = df["cum_mb_per_sec"].rolling(window=window).mean()

            start, end = rolling_events.dropna().iloc[[0, -1]]
            print(f"  Events/sec trend (cumulative, rolling avg window={window}):")
            print(f"    Start:             {start:>10.2f} events/sec")
            print(f"    End:               {end:>10.2f} events/sec")
            print(f"    Change:            {end - start:>10.2f} events/sec")

            start, end = rolling_mb.dropna().iloc[[0, -1]]
            print(f"  MB/sec trend (cumulative, rolling avg window={window}):")
            print(f"    Start:             {start:>10.3f} MB/sec")
            print(f"    End:               {end:>10.3f} MB/sec")
            print(f"    Change:            {end - start:>10.3f} MB/sec")

        # Outliers detection
        print(f"\n{'OUTLIERS (>2 std dev from mean)':-^70}")

        build_time_mean = df["event_build_time_ms"].mean()
        build_time_std = df["event_build_time_ms"].std()
        outlier_threshold = 2

        slow_cut = build_time_mean + outlier_threshold * build_time_std
        slow_events = df[df["event_build_time_ms"] > slow_cut]
        if len(slow_events) > 0:
            print(f"  Slow events (build time > {slow_cut:.2f} ms):")
            for _, row in slow_events.head(5).iterrows():
                event_id = int(row["event_id"])
                build_ms = row["event_build_time_ms"]
                print(f"    Event {event_id:>6d}: {build_ms:>8.2f} ms")
            if len(slow_events) > 5:
                print(f"    ... and {len(slow_events) - 5} more")
        else:
            print("  No significant outliers detected")

        print(f"\n{'=' * 70}\n")

    except FileNotFoundError:
        print(f"Error: CSV file '{csv_file}' not found")
        print("Make sure you've run EventBuilder and event_performance.csv was created")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    csv_file = sys.argv[1] if len(sys.argv) > 1 else "event_performance.csv"
    analyze_performance(csv_file)
