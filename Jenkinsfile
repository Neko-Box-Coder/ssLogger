def bash(command, returnOutput=false)
{
    return sh(script: """#!/bin/bash
        ${command}
        """, 
        returnStdout: returnOutput)
}

def SetGithubStatus(githubToken, context, targetUrl, desc, status, repoOwner, repoName, gitHash)
{
    bash """
        curl -L --fail-with-body \\
        -X POST \\
        -H "Accept: application/vnd.github+json" \\
        -H "Authorization: Bearer ${githubToken}" \\
        -H "X-GitHub-Api-Version: 2022-11-28" \\
        -d '{
            "context": "${context}", 
            "target_url": "${targetUrl}",
            "description": "${desc}",
            "state": "${status}"
        }' \\
        https://api.github.com/repos/${repoOwner}/${repoName}/statuses/${gitHash}
        """
}

def LinuxBuildWithSettings(buildSettings)
{
    cleanWs()
    bash "ls -lah"
    unstash 'source'
    bash "ls -lah"
    bash "mkdir Build"
    bash 'cd ./Build && cmake .. ' + buildSettings + ' && cmake --build . -j 16'
}

def WindowsBuildWithSettings(buildSettings)
{
    cleanWs()
    bat 'dir'
    unstash 'source'
    bat 'dir'
    bat "mkdir Build"
    //2 jobs since it is failing due to memory
    bat 'cd .\\Build && cmake .. ' + buildSettings + ' && cmake --build . -j 2'
}

def PrepareOneStage(stageName, agentLabel, buildSettings, stashName)
{
    return {
        stage(stageName + " (" + agentLabel + ")")
        {
            node(agentLabel)
            {
                try
                {
                    if(agentLabel.equals("windows"))
                        WindowsBuildWithSettings(buildSettings)
                    else if(agentLabel.equals("linux"))
                        LinuxBuildWithSettings(buildSettings)
                    else
                        throw new Exception("Invalid agent label: " + agentLabel)
                    
                    stash(name: agentLabel + "_" + stashName)
                }
                catch(Exception e)
                {
                    FAILED_STAGE = env.STAGE_NAME
                    throw e
                }
            }
        }
    }
}

def ToSnakeCase(str) 
{
   return str.split(' ').join('_').toLowerCase();
}


def ParallelStages()
{
    def stages = [:]
    
    def stageNames = 
        [
            "Static Build", 
            "Shared Build", 
            
            "Call Stack On", 
            "Call Stack Off",
            
            "ASCII On",
            "ASCII Off",
            
            "File Name On",
            "File Name Off",
            "Line Number On",
            "Line Number Off",
            "Func Name On",
            "Func Name Off",
            
            "Date On",
            "Date Off",
            
            "Time On",
            "Time Off",
            
            "Thread Safe On",
            "Thread Safe Off",
            
            "Thread Index On",
            "Thread Index Off",
            
            "Log Mode Console",
            "Log Mode File",
            "Log Mode Console And File",
            
            "Prepend Before Func Name",
            "Prepend Before File Name",
            "Prepend Before Message",
            
            "Log Level None",
            "Log Level Error",
            "Log Level Fatal",
            "Log Level Warning",
            "Log Level Info",
            "Log Level Debug"
        ]
    
    def buildSettings = 
        [
            "-DssLOG_BUILD_TYPE=STATIC", 
            "-DssLOG_BUILD_TYPE=SHARED", 
            
            "-DssLOG_CALL_STACK=ON", 
            "-DssLOG_CALL_STACK=OFF", 
            
            "-DssLOG_LOG_WITH_ASCII=ON", 
            "-DssLOG_LOG_WITH_ASCII=OFF",
            
            "-DssLOG_SHOW_FILE_NAME=ON",
            "-DssLOG_SHOW_FILE_NAME=OFF",
            "-DssLOG_SHOW_LINE_NUM=ON",
            "-DssLOG_SHOW_LINE_NUM=OFF",
            "-DssLOG_SHOW_FUNC_NAME=ON",
            "-DssLOG_SHOW_FUNC_NAME=OFF",
            
            "-DssLOG_SHOW_DATE=ON",
            "-DssLOG_SHOW_DATE=OFF",
            
            "-DssLOG_SHOW_TIME=ON",
            "-DssLOG_SHOW_TIME=OFF",
            
            "-DssLOG_THREAD_SAFE_OUTPUT=ON",
            "-DssLOG_THREAD_SAFE_OUTPUT=OFF",
            
            "-DssLOG_SHOW_THREADS=ON",
            "-DssLOG_SHOW_THREADS=OFF",
            
            "-DssLOG_MODE=CONSOLE",
            "-DssLOG_MODE=FILE",
            "-DssLOG_MODE=CONSOLE_AND_FILE",
            
            "-DssLOG_PREPEND_LOC=BEFORE_FUNC_NAME",
            "-DssLOG_PREPEND_LOC=BEFORE_FILE_NAME",
            "-DssLOG_PREPEND_LOC=BEFORE_MESSAGE",
            
            "-DssLOG_LEVEL=NONE",
            "-DssLOG_LEVEL=FATAL",
            "-DssLOG_LEVEL=ERROR",
            "-DssLOG_LEVEL=WARNING",
            "-DssLOG_LEVEL=INFO",
            "-DssLOG_LEVEL=DEBUG"
        ]
    
    def stashNames = []
    stashNames.add(ToSnakeCase(stageNames.get(0)))
    stashNames.add(ToSnakeCase(stageNames.get(1)))
    for(int i = 2; i < stageNames.size(); i++)
        stashNames.add(ToSnakeCase(stageNames.get(i)) + "_build")
    
    def agentLabels = ["linux", "windows"]
    
    for(int i = 0; i < stageNames.size(); i++)
    {
        for(int j = 0; j < agentLabels.size(); j++)
        {
            stages.put( stageNames.get(i) + " (" + agentLabels.get(j) + ")", 
                        PrepareOneStage(stageNames.get(i), 
                                        agentLabels.get(j), 
                                        buildSettings.get(i),
                                        stashNames.get(i)))
        }
    }
    
    return stages
}

def REPO_OWNER = "Neko-Box-Coder"
def REPO_NAME = "ssLogger"
def TARGET_URL = 'https://github.com/Neko-Box-Coder/ssLogger.git'
def STATUS_CONTEXT_URL = "https://ci.nekoboxcoder.dev/job/ssLogger/${BUILD_NUMBER}/pipeline-graph/"
def FAILED_STAGE = "None"
def TARGET_REF = "refs/heads/main"
def STORE_BUILD = false

pipeline 
{
    agent none
    options { skipDefaultCheckout() }
/*
    External Variables for webhook payload:
    
    Webhook:
        GITHUB_PUSH_REF: $.ref
        GITHUB_PR_ACTION: $.action
        GITHUB_PR_GIT_URL: $.pull_request.head.repo.clone_url
        GITHUB_PR_REF: $.pull_request.head.ref
        GITHUB_PR_REPO_OWNER: $.pull_request.user.login
        GITHUB_PR_REPO_NAME: $.pull_request.head.repo.name
        X-GitHub-Event: (Header)
    
    Trigger:
        $GITHUB_PUSH_REF , $GITHUB_PR_REF , $GITHUB_PR_ACTION
        ^refs/heads/main , , $|^ , .[a-zA-Z/]* , (opened|synchronize)$

    Param:
        TARGET_REF
        STORE_BUILD
*/
    stages 
    {
        stage('Setup') 
        {
            agent none
            steps 
            {
                script
                {
                    echo "env.TARGET_REF: ${env.TARGET_REF}"
                    echo "env.STORE_BUILD: ${env.STORE_BUILD}"
                    echo "Updated Jenkinsfile"
                    
                    if(env.TARGET_REF != null && env.TARGET_REF != '')
                        TARGET_REF = env.TARGET_REF
                    
                    if(env.STORE_BUILD != null && env.STORE_BUILD != '')
                        STORE_BUILD = env.STORE_BUILD.toBoolean()
                    
                    echo "Displaying Webhook Variables:"
                    echo "GITHUB_PUSH_REF: ${env.GITHUB_PUSH_REF}"
                    echo "GITHUB_PR_ACTION: ${env.GITHUB_PR_ACTION}"
                    echo "GITHUB_PR_GIT_URL: ${env.GITHUB_PR_GIT_URL}"
                    echo "GITHUB_PR_REF: ${env.GITHUB_PR_REF}"
                    echo "GITHUB_PR_REPO_OWNER: ${env.GITHUB_PR_REPO_OWNER}"
                    echo "GITHUB_PR_REPO_NAME: ${env.GITHUB_PR_REPO_NAME}"
                    echo "X_GitHub_Event: ${env.X_GitHub_Event}"
                    
                    //Trigger pipeline on push to main
                    if(env.X_GitHub_Event == 'push')
                    {
                        if(env.GITHUB_PUSH_REF != 'refs/heads/main')
                            error('Receiving non main push')
                    }
                    //Trigger pipeline with approval on PR
                    else if(env.X_GitHub_Event == 'pull_request')
                    {
                        if(env.GITHUB_PR_ACTION != 'synchronize' && env.GITHUB_PR_ACTION != 'opened')
                            error('Receiving non relevant PR action')
                        else
                        {
                            if(env.GITHUB_PR_REPO_OWNER == REPO_OWNER)
                            {
                                echo    "env.GITHUB_PR_REPO_OWNER (${env.GITHUB_PR_REPO_OWNER}) is " +
                                        "the same as original REPO_OWNER (${REPO_OWNER})"
                                echo "Skipping approval..."
                            }
                            else
                            {
                                echo    "env.GITHUB_PR_REPO_OWNER (${env.GITHUB_PR_REPO_OWNER}) is " +
                                        " not the same as original REPO_OWNER (${REPO_OWNER})"
                                timeout(time: 30, unit: 'MINUTES')
                                {
                                    input 'Approval this job?'
                                }
                            }
                        }
                        
                        TARGET_REF = env.GITHUB_PR_REF
                        TARGET_URL = env.GITHUB_PR_GIT_URL
                        REPO_OWNER = env.GITHUB_PR_REPO_OWNER
                        REPO_NAME = env.GITHUB_PR_REPO_NAME
                    }
                    //Invalid github event
                    else if(env.X_GitHub_Event != null)
                        error("Invalid github event: ${env.X_GitHub_Event}")
                    
                    echo "TARGET_REF: ${TARGET_REF}"
                    echo "env.TARGET_REF: ${env.TARGET_REF}"
                    echo "TARGET_URL: ${TARGET_URL}"
                    echo "REPO_OWNER: ${REPO_OWNER}"
                    echo "REPO_NAME: ${REPO_NAME}"
                    echo "STATUS_CONTEXT_URL: ${STATUS_CONTEXT_URL}"
                    echo "STORE_BUILD: ${STORE_BUILD}"
                }
            }
        }

        stage('Checkout') 
        {
            agent { label 'linux' }
            steps 
            {
                cleanWs()
                script
                {
                    checkout(
                        [
                            $class: 'GitSCM',
                            branches: [[name: TARGET_REF]],
                            doGenerateSubmoduleConfigurations: true,
                            extensions: scm.extensions + 
                            [[
                                $class: 'SubmoduleOption', 
                                parentCredentials: true,
                                recursiveSubmodules: true
                            ]],
                            userRemoteConfigs: [[url: TARGET_URL]]
                        ]
                    )
                    
                    GIT_HASH = bash("echo \$(git rev-parse --verify HEAD)", true)
                    echo "GITHASH: ${GIT_HASH}"
                    
                    if(!STORE_BUILD)
                    {
                        withCredentials([string(credentialsId: 'github-token', 
                                                variable: 'GITHUB_TOKEN')])
                        {
                            SetGithubStatus('$GITHUB_TOKEN', 
                                            "CI Pipeline", 
                                            STATUS_CONTEXT_URL, 
                                            "Build ${BUILD_NUMBER} stage ${env.STAGE_NAME} started",
                                            "pending",
                                            REPO_OWNER,
                                            REPO_NAME,
                                            GIT_HASH)
                        }
                    }
                    
                    stash 'source'
                }
            }
            post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
        }

        stage('Build') 
        {
            steps
            {
                script
                {
                    parallel ParallelStages()
                }
            }
            //NOTE: We use Debug builds for now even for release.
        }

        stage('Test') 
        {
            parallel 
            {
                stage('Linux Header Only Test') 
                {
                    agent { label 'linux' }
                    steps 
                    {
                        cleanWs()
                        bash "ls -lah"
                        unstash 'linux_static_build'
                        bash "ls -lah"
                        bash "ls -lah ./Build"
                        bash "chmod -R +x ./Build"
                        bash "chmod +x ./CI/RunHeaderOnlyTests.sh"
                        bash "cd ./CI && ./RunHeaderOnlyTests.sh"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Linux Static Test') 
                {
                    agent { label 'linux' }
                    steps 
                    {
                        cleanWs()
                        bash "ls -lah"
                        unstash 'linux_static_build'
                        bash "ls -lah"
                        bash "ls -lah ./Build"
                        bash "chmod -R +x ./Build"
                        bash "chmod +x ./CI/RunSourceTests.sh"
                        bash "cd ./CI && ./RunSourceTests.sh"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Linux Shared Test') 
                {
                    agent { label 'linux' }
                    steps 
                    {
                        cleanWs()
                        bash "ls -lah"
                        unstash 'linux_shared_build'
                        bash "ls -lah"
                        bash "ls -lah ./Build"
                        bash "chmod -R +x ./Build"
                        bash "chmod +x ./CI/RunSourceTests.sh"
                        bash "cd ./CI && ./RunSourceTests.sh"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Linux Log To File Test') 
                {
                    agent { label 'linux' }
                    steps 
                    {
                        cleanWs()
                        bash "ls -lah"
                        unstash 'linux_log_mode_file_build'
                        bash "ls -lah"
                        bash "ls -lah ./Build"
                        bash "chmod -R +x ./Build"
                        bash "chmod +x ./CI/RunSourceTests.sh"
                        bash "cd ./CI && ./RunSourceTests.sh"
                        bash "ls -lah ./CI"
                        bash "cd ./CI && ls *.txt >/dev/null || exit 1"
                        bash "find ./CI -maxdepth 1 -type f -name '*.txt' -exec sh -c \"echo '{}'; cat '{}'\" \\;"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Linux Log To Both Console File Test')
                {
                    agent { label 'linux' }
                    steps 
                    {
                        cleanWs()
                        bash "ls -lah"
                        unstash 'linux_log_mode_console_and_file_build'
                        bash "ls -lah"
                        bash "ls -lah ./Build"
                        bash "chmod -R +x ./Build"
                        bash "chmod +x ./CI/RunSourceTests.sh"
                        bash "cd ./CI && ./RunSourceTests.sh"
                        bash "ls -lah ./CI"
                        bash "cd ./CI && ls *.txt >/dev/null || exit 1"
                        bash "find ./CI -maxdepth 1 -type f -name '*.txt' -exec sh -c \"echo '{}'; cat '{}'\" \\;"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                
                stage('Windows Header Only Test') 
                {
                    agent { label 'windows' }
                    steps 
                    {
                        cleanWs()
                        bat 'dir'
                        unstash 'windows_static_build'
                        bat 'dir'
                        bat 'cd .\\CI && .\\RunHeaderOnlyTests.bat'
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Windows Static Test') 
                {
                    agent { label 'windows' }
                    steps 
                    {
                        cleanWs()
                        bat 'dir'
                        unstash 'windows_static_build'
                        bat 'dir'
                        bat 'cd .\\CI && .\\RunSourceTests.bat'
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Windows Shared Test') 
                {
                    agent { label 'windows' }
                    steps 
                    {
                        cleanWs()
                        bat 'dir'
                        unstash 'windows_shared_build'
                        bat 'dir'
                        bat 'cd .\\CI && .\\RunSourceTests.bat'
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Windows Log To File Test') 
                {
                    agent { label 'windows' }
                    steps 
                    {
                        cleanWs()
                        bat 'dir'
                        unstash 'windows_log_mode_file_build'
                        bat 'dir'
                        bat 'cd .\\CI && .\\RunSourceTests.bat'
                        bat "dir .\\Build\\SourceTests\\Debug"
                        bat "cd .\\Build\\SourceTests\\Debug && dir /a-d *.txt >nul 2>&1"
                        bat "type .\\Build\\SourceTests\\Debug\\*.txt"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
                stage('Windows Both Console File Test') 
                {
                    agent { label 'windows' }
                    steps 
                    {
                        cleanWs()
                        bat 'dir'
                        unstash 'windows_log_mode_console_and_file_build'
                        bat 'dir'
                        bat 'cd .\\CI && .\\RunSourceTests.bat'
                        bat "dir .\\Build\\SourceTests\\Debug"
                        bat "cd .\\Build\\SourceTests\\Debug && dir /a-d *.txt >nul 2>&1"
                        bat "type .\\Build\\SourceTests\\Debug\\*.txt"
                    }
                    post { failure { script { FAILED_STAGE = env.STAGE_NAME } } }
                }
            }
        }
        
        stage('Notify')
        {
            agent { label 'linux' }
            when { expression { STORE_BUILD == false } }
            steps 
            {
                cleanWs()
                withCredentials([string(credentialsId: 'github-token', variable: 'GITHUB_TOKEN')])
                {
                    SetGithubStatus('$GITHUB_TOKEN', 
                                    "CI Pipeline", 
                                    STATUS_CONTEXT_URL, 
                                    "Build ${BUILD_NUMBER} Pipeline passed",
                                    "success",
                                    REPO_OWNER,
                                    REPO_NAME,
                                    GIT_HASH)
                }
            }
        }
    } //stages
    
    post 
    {
        failure 
        {
            node('linux')
            {
                script
                {
                    if(!STORE_BUILD)
                    {
                        withCredentials([string(credentialsId: 'github-token', variable: 'GITHUB_TOKEN')])
                        {
                            SetGithubStatus('$GITHUB_TOKEN', 
                                            "CI Pipeline", 
                                            STATUS_CONTEXT_URL, 
                                            "Build ${BUILD_NUMBER} Stage ${FAILED_STAGE} failed",
                                            "failure",
                                            REPO_OWNER,
                                            REPO_NAME,
                                            GIT_HASH)
                        }
                    }
                }
            }
        }
    }
} //pipeline
