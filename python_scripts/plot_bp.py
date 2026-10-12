import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("csvs/patterns.csv")
policies = ["LRU", "LFU", "2Q", "LIRS", "ARC"]
cols = policies + ["OPT"]
n_runs = df.seed.nunique()


def ci95(s):
    return 1.96 * s.std(ddof=1) / np.sqrt(len(s))


order = list(dict.fromkeys(df.pattern))
mean = df.groupby("pattern")[cols].mean().loc[order]
ci = df.groupby("pattern")[cols].agg(ci95).loc[order]

ax = mean.plot(kind="bar", yerr=ci, capsize=2, figsize=(10, 5), width=0.8)
ax.set_ylabel("hit ratio")
ax.set_ylim(0, 1)
ax.set_title(f"Hit ratio по паттернам, среднее ± 95% ДИ ({n_runs} сидов)")
ax.tick_params(axis="x", rotation=0)
ax.legend(ncol=6, loc="upper center")
plt.tight_layout()
plt.savefig("images_new/patterns_hr.png", dpi=150)

rel = df[policies].div(df["OPT"], axis=0)
rel["pattern"] = df["pattern"]
rel_mean = rel.groupby("pattern")[policies].mean().loc[order]
rel_ci = rel.groupby("pattern")[policies].agg(ci95).loc[order]

ax2 = rel_mean.plot(kind="bar", yerr=rel_ci, capsize=2, figsize=(10, 5), width=0.8)
ax2.axhline(1.0, color="red", linestyle="--", label="OPT")
ax2.set_ylabel("hit ratio / OPT")
ax2.set_ylim(0, 1.1)
ax2.set_title(f"Доля от оптимума, среднее ± 95% ДИ ({n_runs} сидов)")
ax2.tick_params(axis="x", rotation=0)
ax2.legend(ncol=6, loc="upper center")
plt.tight_layout()
plt.savefig("images_new/patterns_vs_opt.png", dpi=150)

print("successfully done")
