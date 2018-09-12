function v = helics_pending_iterative_time_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230880);
  end
  v = vInitialized;
end
