function v = helics_state_pending_iterative_time()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856883);
  end
  v = vInitialized;
end
