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
    core.info(`Using workflow run ID from payload: ${runId}`);
  } else {
    // Look up the most recent completed run of the build workflow on main
    core.info('Looking up latest completed build workflow run...');
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
      core.info(`Found workflow run ID: ${runId}`);
    } else {
      throw new Error('No completed workflow run found for build-and-release.yml on branch main');
    }
  }

  core.info(`Fetching artifacts from run ID: ${runId}`);
  const artifacts = await actionsApi.listWorkflowRunArtifacts({
    owner: context.repo.owner,
    repo: context.repo.repo,
    run_id: runId,
  });

  core.info(`Found ${artifacts.data.artifacts.length} artifacts`);
  
  for (const artifact of artifacts.data.artifacts) {
    core.info(`Downloading artifact: ${artifact.name} (ID: ${artifact.id})`);
    
    const download = await actionsApi.downloadArtifact({
      owner: context.repo.owner,
      repo: context.repo.repo,
      artifact_id: artifact.id,
      archive_format: 'zip',
    });

    const buffer = Buffer.from(download.data);
    const filename = `${artifact.name}.zip`;
    fs.writeFileSync(filename, buffer);
    core.info(`Saved artifact to ${filename} (${buffer.length} bytes)`);
  }
  
  core.info('All artifacts downloaded successfully');
})();
