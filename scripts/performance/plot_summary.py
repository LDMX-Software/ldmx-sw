"""Plot performance data from summary and by_event csv files across runs"""

import argparse
import os
import textwrap

import matplotlib.pyplot as plt
import pandas as pd


DATA_DIR = "scripts/performance/data"
OUT_DIR = "scripts/performance/plots"


def plot_stacked_bar(
    runs, max_events, mode, num_top, sorted_processors, sum_others=True
):
    """Plot Mean Time Fraction Per Processor stacked bar chart using time fraction
    of __ALL__ duration per processor averaged across runs"""
    fig, ax = plt.subplots(figsize=(6, 8))

    top_processors = sorted_processors.iloc[:num_top]
    bottom_processors = sorted_processors.iloc[num_top:]

    bottom = 0
    for processor, fraction in top_processors.items():
        ax.bar(0, fraction, bottom=bottom, label=processor)
        bottom += fraction

    if sum_others and len(bottom_processors) > 0:
        bottom_sum = bottom_processors.sum()
        ax.bar(
            0,
            bottom_sum,
            bottom=bottom,
            label=f"Other {len(bottom_processors)} processors",
            color="grey",
        )

    ax.legend(loc="upper right", fontsize="small")

    ax.set_xlabel(f"Max Events = {max_events}")
    ax.set_ylabel("Mean Time Fraction of __ALL__ Duration")
    ax.set_title(
        f"Mean Time Fraction per Processor for {mode}, for {len(runs)} runs, "
        f"{max_events} events"
    )

    plt.tight_layout()
    plt.savefig(
        f"{OUT_DIR}/time_fraction_per_proc_{mode}_{len(runs)}_runs_{max_events}_events.png"
    )
    plt.close(fig)


def load_runs_dist(runs, max_events, mode):
    """Load the run data from each by_event CSV file and return DataFrame for per-event
    duration, excluding the first event. The DataFrame is organized by event_id,
    processor, and duration. Also returns DataFrame of the mean duration of the
    first event per processor"""

    run_data = []
    first_durs = []
    for run in runs:
        path = f"{DATA_DIR}/run_{run}/by_event_{mode}_{max_events}.csv"
        f = pd.read_csv(path)
        duration_df = f.pivot(index="event_id", columns="processor", values="duration")
        run_data.append(duration_df.iloc[1:])
        first_durs.append(duration_df.iloc[0])

    mean_first_durs = pd.DataFrame(first_durs).mean()
    return pd.concat(run_data), mean_first_durs


def plot_dist(runs, max_events, mode, num_top, sorted_processors, sum_others=True):
    """Plot the distribution of event duration per processor across multiple runs,
    including a point to indicate the mean duration of the first event per processor.
    The whiskers show data from the 1st to the 99th percentile, so 98% of the events
    lie within the whiskers. The maximum duration of an event, excluding first event,
    for each processor is plotted to indicate the highest duration outlier since
    showfliers is set to False."""

    run_durations, first_durs = load_runs_dist(runs, max_events, mode)
    fig, ax = plt.subplots(figsize=(10, 6))

    top_processors = list(sorted_processors.index[:num_top])
    bottom_processors = list(sorted_processors.index[num_top:])

    data = run_durations[top_processors].copy()
    first_data = first_durs[top_processors].copy()
    labels = list(top_processors)

    if sum_others and len(bottom_processors) > 0:
        data["__OTHERS__"] = run_durations[bottom_processors].sum(axis=1)
        first_data["__OTHERS__"] = first_durs[bottom_processors].sum()
        labels.append(f"Other {len(bottom_processors)} processors")

    ax.boxplot(
        data,
        orientation="horizontal",
        tick_labels=labels,
        whis=(1, 99),
        showfliers=False,
    )

    ax.scatter(
        first_data,
        range(1, len(first_data) + 1),
        marker="x",
        color="red",
        label="Mean First Completed Event Duration",
    )
    ax.scatter(
        data.max(),
        range(1, len(data.columns) + 1),
        marker=".",
        color="blue",
        label="Maximum Duration, excluding first event",
    )

    caption = (
        "Boxes show quartiles of event durations for each run, excluding the first"
        " duration. Whiskers indicate 1st-99th percentile. Processors are ordered"
        " by their mean fraction of total duration."
    )

    ax.text(
        0.5,
        -0.20,
        textwrap.fill(caption, 110),
        transform=ax.transAxes,
        ha="center",
        va="top",
        fontsize=8,
    )
    ax.legend(loc="upper center", bbox_to_anchor=(0.5, -0.11), ncol=2, fontsize="small")
    ax.set_xscale("log")
    ax.set_xlabel("Duration (s)")
    ax.set_ylabel("Processor")
    ax.invert_yaxis()
    ax.set_title(
        f"Per-Event Duration Distribution for {mode} across {len(runs)} runs, "
        f"{max_events} events"
    )
    plt.tight_layout()

    plt.savefig(
        f"{OUT_DIR}/per_event_distribution_{mode}_{len(runs)}_runs_{max_events}_events.png",
        bbox_inches="tight",
    )
    plt.close(fig)


def load_runs_cumul(runs, max_events, mode):
    """Load the run data from each by_event CSV file, return DataFrame for
    cumulative duration, organized by run and event."""
    run_data = []
    for run in runs:
        path = f"{DATA_DIR}/run_{run}/by_event_{mode}_{max_events}.csv"
        f = pd.read_csv(path)

        df = f.pivot(index="event_id", columns="processor", values="duration")
        cumul_df = df.cumsum().reset_index(drop=True)
        run_data.append(cumul_df)

    # Filter data so x-range is from the run with the lowest completed events
    min_length = min(len(run) for run in run_data)
    run_data = [run.iloc[:min_length] for run in run_data]

    return pd.concat(run_data, keys=range(len(run_data)), names=["run", "event"])


def plot_cumulative(
    runs, max_events, mode, num_top, sorted_processors, sum_others=True
):
    """Plot the cumulative duration over events of each processor, using mean to
    plot an average line and std dev at each event to create a band around the line."""
    run_cumuls = load_runs_cumul(runs, max_events, mode)

    top_processors = sorted_processors.index[:num_top]
    bottom_processors = sorted_processors.index[num_top:]

    if sum_others and len(bottom_processors) > 0:
        run_cumuls["__OTHERS__"] = run_cumuls[bottom_processors].sum(axis=1)

    mean_df = run_cumuls.groupby(level="event").mean()
    std_df = run_cumuls.groupby(level="event").std()

    def draw_plot(ax, processor_name, color=None, label=None, linestyle="-", alpha=0.2):
        (line,) = ax.plot(
            mean_df.index + 1,
            mean_df[processor_name],
            color=color,
            label=label or processor_name,
            linestyle=linestyle,
        )
        ax.fill_between(
            mean_df.index + 1,
            mean_df[processor_name] - std_df[processor_name],
            mean_df[processor_name] + std_df[processor_name],
            color=line.get_color(),
            alpha=alpha,
        )

    fig, ax = plt.subplots(figsize=(10, 6))

    draw_plot(ax, "__ALL__", color="black", label="All Processors", linestyle="--")

    for name in top_processors:
        draw_plot(ax, name)
    if sum_others and len(bottom_processors) > 0:
        draw_plot(
            ax,
            "__OTHERS__",
            color="gray",
            label=f"Other {len(bottom_processors)} Processors",
            linestyle=":",
        )

    ax.set_xlabel("Number of Completed Events")
    ax.set_ylabel("Cumulative duration (s)")
    ax.set_title(
        f"Mean Cumulative Duration per Processor for {mode} across {len(runs)} runs, "
        f"{max_events} events"
    )
    ax.legend()
    plt.tight_layout()
    plt.savefig(
        f"{OUT_DIR}/cumulative_duration_per_proc_{mode}_{len(runs)}_runs_{max_events}_events.png"
    )
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser(description="plot_summary")
    ap.add_argument("mode", choices=["sim", "recon", "full"])
    ap.add_argument("--runs", type=int, nargs="+", help="runs to plot")
    ap.add_argument("--max-events", type=int, default=1000)
    ap.add_argument(
        "--min-events",
        type=int,
        default=1000,
        help="drop runs that completed fewer events than this",
    )
    args = ap.parse_args()

    runs = args.runs
    if not runs:
        raise SystemExit("No runs were inputted to the program")

    mode, max_events, min_events = args.mode, args.max_events, args.min_events

    if min_events > max_events or min_events < 0.95 * max_events:
        min_events = 0.95 * max_events

    def filter_runs(runs, min_events, max_events, mode):
        """Filter runs by completed events compared to min_events, check if csv
        files are valid, return list of kept runs"""
        kept_runs = []
        for run in runs:
            summary = f"{DATA_DIR}/run_{run}/summary_{mode}_{max_events}.csv"
            by_event = f"{DATA_DIR}/run_{run}/by_event_{mode}_{max_events}.csv"
            if not os.path.exists(summary) or not os.path.exists(by_event):
                raise SystemExit(
                    f"Cannot find data for run {run}, expected to be found in "
                    f"{summary} and {by_event}."
                )
            f = pd.read_csv(summary)
            completed = f["completed_event_count"].iloc[0]
            if completed < min_events:
                print(
                    f"Run {run} dropped for completing {completed} events, below the "
                    f"minimum ({min_events})."
                )
            else:
                kept_runs.append(run)
        if not kept_runs:
            raise SystemExit(f"No runs met threshold for minimum events = {min_events}")
        return kept_runs

    def sort_processors(runs, max_events, mode):
        """Sort processors from highest to lowest duration, return sorted processors
        list"""
        time_fractions = []
        for run in runs:
            f = pd.read_csv(f"{DATA_DIR}/run_{run}/summary_{mode}_{max_events}.csv")
            time_fractions.append(f.set_index("processor")["all_fraction"])

        time_fractions = pd.concat(time_fractions)
        mean_fractions = time_fractions.groupby(level=0).mean()
        mean_fractions = mean_fractions.drop("__ALL__").sort_values(ascending=False)
        return mean_fractions

    runs = filter_runs(runs, min_events, max_events, mode)
    sorted_processors = sort_processors(runs, max_events, mode)

    os.makedirs(OUT_DIR, exist_ok=True)

    # Set number of processors to be on plot depending on mode
    num_highest_processors_dist = 12

    if mode == "full" or mode == "recon":
        num_highest_processors_frac = 7
        num_highest_processors_cumul = 4
    else:
        num_highest_processors_frac = 1
        num_highest_processors_cumul = 4

    plot_stacked_bar(
        runs,
        max_events,
        mode,
        num_highest_processors_frac,
        sorted_processors,
        sum_others=True,
    )
    plot_dist(
        runs,
        max_events,
        mode,
        num_highest_processors_dist,
        sorted_processors,
        sum_others=True,
    )
    plot_cumulative(
        runs,
        max_events,
        mode,
        num_highest_processors_cumul,
        sorted_processors,
        sum_others=True,
    )


if __name__ == "__main__":
    main()
