function v = FORWARD_COMPUTE_FLAG()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 11);
  end
  v = vInitialized;
end
