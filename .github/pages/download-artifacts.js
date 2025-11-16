const fs = require('fs');
const path = require('path');

// Helper to determine the workflow run id.
// When this job is triggered by a workflow_run event the run id is
// available at context.payload.workflow_run.id. When triggered by a
// push event, that value is undefined, so we find the latest
// completed run of the build workflow and use its id.
const actionsApi = github.rest.actions;

(async () => {
  let runId = null;
  if (context.payload && context.payload.workflow_run && context.payload.workflow_run.id) {
    runId = context.payload.workflow_run.id;
  } else {
    // Look up the most recent completed run of the build workflow on main
    const runs = await actionsApi.listWorkflowRuns({
      owner: context.repo.owner,
      repo: context.repo.repo,
      workflow_id: 'build-and-release.yml',
      branch: 'main',
      status: 'completed',
      per_page: 1,
    });

    if (runs && runs.data && runs.data.workflow_runs && runs.data.workflow_runs.length > 0) {
      runId = runs.data.workflow_runs[0].id;
    } else {
      throw new Error('No completed workflow run found for build-and-release.yml on branch main');
    }
  }

  const artifacts = await actionsApi.listWorkflowRunArtifacts({
    owner: context.repo.owner,
    repo: context.repo.repo,
    run_id: runId,
  });

  for (const artifact of artifacts.data.artifacts) {
    const download = await actionsApi.downloadArtifact({
      owner: context.repo.owner,
      repo: context.repo.repo,
      artifact_id: artifact.id,
      archive_format: 'zip',
    });

    const buffer = Buffer.from(download.data);
    fs.writeFileSync(`${artifact.name}.zip`, buffer);
  }
})();
