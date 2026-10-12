import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("csvs/results.csv")


def ci95(s):
    return 1.96 * s.std(ddof=1) / np.sqrt(len(s))


stats = df.groupby("config", sort=False).agg(
    hr=("hr", "mean"), hr_ci=("hr", ci95),
    cost=("cost", "mean"), cost_ci=("cost", ci95),
    h1=("h1", "mean"), h2=("h2", "mean"), miss=("miss", "mean"),
)

opt_hr = stats.loc["OPT", "hr"]
opt_ci = stats.loc["OPT", "hr_ci"]
stats = stats.drop("OPT")
stats[["l1", "l2"]] = stats.index.to_series().str.split(" \\+ ", expand=True)
stats["config"] = stats.index

names = list(dict.fromkeys(stats.l1))
n_runs = df.seed.nunique()

fig, axes = plt.subplots(1, 2, figsize=(14, 5.5))

for ax, col, title, fmt in [(axes[0], "hr", "Hit ratio (выше лучше)", "{:.3f}\n±{:.3f}"),
                            (axes[1], "cost", "Cost (ниже лучше)", "{:.0f}\n±{:.0f}")]:
    m = stats.pivot(index="l1", columns="l2", values=col).loc[names, names]
    c = stats.pivot(index="l1", columns="l2", values=col + "_ci").loc[names, names]
    im = ax.imshow(m.values, cmap="viridis" if col == "hr" else "viridis_r")
    ax.set_xticks(range(len(names)), names)
    ax.set_yticks(range(len(names)), names)
    ax.set_xlabel("L2")
    ax.set_ylabel("L1")
    ax.set_title(f"{title}, среднее ± 95% ДИ, {n_runs} сидов")
    for i in range(len(names)):
        for j in range(len(names)):
            ax.text(j, i, fmt.format(m.values[i, j], c.values[i, j]),
                    ha="center", va="center", color="white", fontsize=7)
    fig.colorbar(im, ax=ax)

fig.tight_layout()
fig.savefig("images_new/heatmaps.png", dpi=150)

d = stats.sort_values("cost")
fig2, ax2 = plt.subplots(figsize=(13, 5))
ax2.bar(d.config, d.h1, label="L1 hits")
ax2.bar(d.config, d.h2, bottom=d.h1, label="L2 hits")
ax2.bar(d.config, d.miss, bottom=d.h1 + d.h2, label="misses")
ax2.set_ylabel("запросов (среднее)")
ax2.set_title("Распределение запросов (слева лучшие по cost)")
ax2.tick_params(axis="x", rotation=90)
ax2.legend()
fig2.tight_layout()
fig2.savefig("images_new/breakdown.png", dpi=150)

d = stats.sort_values("hr", ascending=False)
fig3, ax3 = plt.subplots(figsize=(13, 5))
ax3.bar(d.config, d.hr, yerr=d.hr_ci, capsize=3, ecolor="black")
ax3.axhline(opt_hr, color="red", linestyle="--", label=f"OPT = {opt_hr:.3f}")
ax3.axhspan(opt_hr - opt_ci, opt_hr + opt_ci, color="red", alpha=0.15)
ax3.set_ylabel("hit ratio")
ax3.set_title(f"Hit ratio, среднее ± 95% ДИ ({n_runs} сидов)")
ax3.tick_params(axis="x", rotation=90)
ax3.legend()
fig3.tight_layout()
fig3.savefig("images_new/hit_ratio.png", dpi=150)

print("successfully done")
