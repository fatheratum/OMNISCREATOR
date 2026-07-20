/**
 * OMNIBRAIN GODHEAD — JavaScript Bridge
 * Used by godhead-dashboard (WebGPU / TUI) to talk to the Python economic core.
 *
 * Current: Spawns Python subprocess or uses shared memory / WebSocket (local only).
 * Future: WebAssembly port or native addon.
 */

const { spawn } = require('child_process');
const path = require('path');

class OMNIBRAINBridge {
    constructor(pythonPath = 'python3') {
        this.pythonPath = pythonPath;
        this.scriptDir = path.join(__dirname, '..', 'python');
    }

    async call(method, args = []) {
        return new Promise((resolve, reject) => {
            const py = spawn(this.pythonPath, [
                path.join(this.scriptDir, 'reserve_nexus.py') // or a dedicated bridge entrypoint
            ], { stdio: ['pipe', 'pipe', 'pipe'] });

            let output = '';
            py.stdout.on('data', (data) => { output += data.toString(); });
            py.stderr.on('data', (data) => { /* log */ });

            py.on('close', (code) => {
                if (code === 0) {
                    try {
                        resolve(JSON.parse(output.trim()));
                    } catch (e) {
                        resolve({ raw: output.trim() });
                    }
                } else {
                    reject(new Error(`Python exited with code ${code}`));
                }
            });

            // In real implementation: send JSON-RPC or command to a long-running Python daemon
            py.stdin.write(JSON.stringify({ method, args }) + '\n');
            py.stdin.end();
        });
    }

    async getSolvencyIndex() {
        return this.call('get_solvency_index');
    }

    async acquireDeposit(organismId, value) {
        return this.call('acquire_deposit', [organismId, value]);
    }
}

module.exports = { OMNIBRAINBridge };
