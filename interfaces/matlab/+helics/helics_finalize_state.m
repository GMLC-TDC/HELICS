function v = helics_finalize_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230875);
  end
  v = vInitialized;
end
