function v = helics_state_finalize()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230875);
  end
  v = vInitialized;
end
