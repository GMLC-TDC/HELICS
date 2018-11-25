function v = helics_state_pending_iterative_time()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183131);
  end
  v = vInitialized;
end
