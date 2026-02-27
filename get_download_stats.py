#!/usr/bin/env python3
"""
Script to fetch and display GitHub release download statistics
for the pico-infonesPlus repository.

Usage:
  ./get_download_stats.py                    - Fetch from GitHub API
  ./get_download_stats.py releases.json      - Use local JSON file
  
For fetching from GitHub with authentication:
  export GITHUB_TOKEN=your_token_here
  ./get_download_stats.py
"""

import urllib.request
import json
import sys
import os
from datetime import datetime

# Repository information
OWNER = "fhoedemakers"
REPO = "pico-infonesPlus"
API_URL = f"https://api.github.com/repos/{OWNER}/{REPO}/releases"


def fetch_releases_from_api():
    """Fetch all releases from GitHub API"""
    try:
        # Add User-Agent header to avoid rate limiting
        req = urllib.request.Request(API_URL)
        req.add_header('User-Agent', 'pico-infonesPlus-download-stats/1.0')
        req.add_header('Accept', 'application/vnd.github.v3+json')
        
        # Add authentication token if available
        github_token = os.environ.get('GITHUB_TOKEN')
        if github_token:
            req.add_header('Authorization', f'token {github_token}')
        
        with urllib.request.urlopen(req) as response:
            if response.status != 200:
                print(f"Error: API returned status code {response.status}")
                return None
            data = response.read()
            return json.loads(data)
    except urllib.error.HTTPError as e:
        print(f"Error fetching releases: HTTP {e.code} - {e.reason}")
        if e.code == 403:
            print("Note: GitHub API rate limit may be exceeded.")
            print("Try setting GITHUB_TOKEN environment variable or use a local JSON file.")
        return None
    except Exception as e:
        print(f"Error fetching releases: {e}")
        return None


def fetch_releases_from_file(filename):
    """Load releases from a local JSON file"""
    try:
        with open(filename, 'r') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading file {filename}: {e}")
        return None


def fetch_releases(source=None):
    """Fetch releases from API or file"""
    if source and os.path.isfile(source):
        print(f"Loading release data from file: {source}")
        return fetch_releases_from_file(source)
    else:
        print("Fetching release data from GitHub API...")
        return fetch_releases_from_api()


def format_number(num):
    """Format number with thousands separator"""
    return f"{num:,}"


def print_release_stats(releases):
    """Print download statistics for all releases"""
    if not releases:
        print("No releases found.")
        return

    print("=" * 100)
    print(f"Release Download Statistics for {OWNER}/{REPO}")
    print("=" * 100)
    print()

    total_downloads = 0
    
    for release in releases:
        tag = release.get("tag_name", "Unknown")
        name = release.get("name", "Unnamed")
        published = release.get("published_at", "Unknown")
        assets = release.get("assets", [])
        
        # Calculate total downloads for this release
        release_downloads = sum(asset.get("download_count", 0) for asset in assets)
        total_downloads += release_downloads
        
        # Format date
        try:
            pub_date = datetime.strptime(published, "%Y-%m-%dT%H:%M:%SZ")
            pub_date_str = pub_date.strftime("%Y-%m-%d")
        except:
            pub_date_str = published
        
        print(f"Release: {tag} - {name}")
        print(f"Published: {pub_date_str}")
        print(f"Total downloads: {format_number(release_downloads)}")
        
        if assets:
            print(f"Assets ({len(assets)}):")
            for asset in assets:
                asset_name = asset.get("name", "Unknown")
                downloads = asset.get("download_count", 0)
                size_mb = asset.get("size", 0) / (1024 * 1024)
                print(f"  - {asset_name:60s} {format_number(downloads):>10s} downloads ({size_mb:.2f} MB)")
        else:
            print("  No assets found")
        
        print("-" * 100)
        print()
    
    print("=" * 100)
    print(f"TOTAL DOWNLOADS ACROSS ALL RELEASES: {format_number(total_downloads)}")
    print("=" * 100)


def print_summary(releases):
    """Print a summary of download statistics"""
    if not releases:
        return
    
    print("\n")
    print("=" * 100)
    print("SUMMARY")
    print("=" * 100)
    
    total_releases = len(releases)
    total_downloads = sum(
        sum(asset.get("download_count", 0) for asset in release.get("assets", []))
        for release in releases
    )
    total_assets = sum(len(release.get("assets", [])) for release in releases)
    
    print(f"Total Releases: {total_releases}")
    print(f"Total Assets: {total_assets}")
    print(f"Total Downloads: {format_number(total_downloads)}")
    
    if total_releases > 0:
        avg_downloads = total_downloads / total_releases
        print(f"Average Downloads per Release: {format_number(int(avg_downloads))}")
    
    # Find most downloaded release
    most_downloaded = max(
        releases,
        key=lambda r: sum(asset.get("download_count", 0) for asset in r.get("assets", [])),
        default=None
    )
    
    if most_downloaded:
        most_downloads = sum(
            asset.get("download_count", 0) for asset in most_downloaded.get("assets", [])
        )
        print(f"Most Downloaded Release: {most_downloaded.get('tag_name')} ({format_number(most_downloads)} downloads)")
    
    print("=" * 100)


def main():
    """Main function"""
    # Check if a file was provided as argument
    source = sys.argv[1] if len(sys.argv) > 1 else None
    
    releases = fetch_releases(source)
    
    if releases is None:
        print("\nTip: You can also run this script with a local JSON file:")
        print(f"  python3 {sys.argv[0]} releases.json")
        sys.exit(1)
    
    print()
    print_release_stats(releases)
    print_summary(releases)


if __name__ == "__main__":
    main()
