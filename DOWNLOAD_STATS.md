# Release Download Statistics

This document shows how to get download statistics for all releases of the pico-infonesPlus project.

## Quick Start

Run the Python script to fetch and display download statistics:

```bash
python3 get_download_stats.py
```

## Requirements

- Python 3.x (no additional packages required - uses only standard library)
- Internet connection to fetch from GitHub API
- Optional: GitHub personal access token for higher API rate limits

## Usage

### Method 1: Fetch from GitHub API (Recommended)

```bash
python3 get_download_stats.py
```

This will fetch release data directly from GitHub and display comprehensive download statistics.

### Method 2: With GitHub Token (For Higher Rate Limits)

If you encounter rate limiting, set a GitHub token:

```bash
export GITHUB_TOKEN=your_github_token_here
python3 get_download_stats.py
```

### Method 3: Use a Local JSON File

You can also save release data to a JSON file and use it offline:

```bash
# First, fetch and save the data
python3 get_download_stats.py > releases_data.json 2>&1

# Or use curl to fetch raw data
curl -H "Accept: application/vnd.github.v3+json" \
     https://api.github.com/repos/fhoedemakers/pico-infonesPlus/releases \
     > releases_data.json

# Then use the local file
python3 get_download_stats.py releases_data.json
```

## Output

The script provides:

1. **Per-Release Statistics**: Detailed breakdown of each release showing:
   - Release tag and name
   - Publication date
   - Total downloads for the release
   - Individual asset download counts and sizes

2. **Summary Statistics**:
   - Total number of releases
   - Total number of assets across all releases
   - Total downloads across all releases
   - Average downloads per release
   - Most downloaded release

## Example Output

```
====================================================================================================
Release Download Statistics for fhoedemakers/pico-infonesPlus
====================================================================================================

Release: v0.37 - v0.37
Published: 2025-02-10
Total downloads: 725
Assets (16):
  - piconesPlus_Pico_arm.uf2                                        261 downloads (1.13 MB)
  - piconesPlus_PCB_Pico_arm.uf2                                    137 downloads (1.13 MB)
  - pico_nesPCB_v2.1.zip                                             80 downloads (0.08 MB)
  ...

----------------------------------------------------------------------------------------------------

====================================================================================================
SUMMARY
====================================================================================================
Total Releases: 36
Total Assets: 400+
Total Downloads: 50,000+
Average Downloads per Release: 1,400
Most Downloaded Release: v0.28 (10,234 downloads)
====================================================================================================
```

## Automation

You can automate download statistics tracking:

### GitHub Actions Workflow

Create `.github/workflows/download-stats.yml`:

```yaml
name: Weekly Download Statistics

on:
  schedule:
    - cron: '0 0 * * 0'  # Run every Sunday at midnight
  workflow_dispatch:  # Allow manual trigger

jobs:
  stats:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Get Download Stats
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
        run: |
          python3 get_download_stats.py > download_stats_$(date +%Y%m%d).txt
          
      - name: Upload Stats
        uses: actions/upload-artifact@v4
        with:
          name: download-stats
          path: download_stats_*.txt
```

### Cron Job

Add to your crontab for weekly reports:

```bash
# Every Sunday at 9 AM
0 9 * * 0 cd /path/to/pico-infonesPlus && python3 get_download_stats.py > stats_$(date +\%Y\%m\%d).txt
```

## API Rate Limits

GitHub API has rate limits:
- **Unauthenticated**: 60 requests per hour
- **Authenticated**: 5,000 requests per hour

The script makes only 1 API request to fetch all releases, so it's well within limits.

## Troubleshooting

### HTTP 403 Error

If you get a 403 Forbidden error:
1. Check if you've exceeded the rate limit
2. Use a GitHub token (see Method 2 above)
3. Wait an hour for the rate limit to reset

### No Data Displayed

- Verify internet connection
- Check that the repository name is correct
- Try using a local JSON file (Method 3)

## Additional Resources

- [GitHub REST API Documentation](https://docs.github.com/en/rest/releases/releases)
- [Creating a GitHub Personal Access Token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token)

## Contributing

If you find issues or have suggestions for the download statistics tool, please open an issue or submit a pull request.
